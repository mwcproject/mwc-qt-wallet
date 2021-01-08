// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Json.h"
#include <QJsonDocument>
#include <QDebug>
#include "Files.h"
#include "stringutils.h"
#include <QJsonArray>
#include "../state/state.h"
#include "../wallet/wallet.h"

namespace util {

// Return emoty if not able to parse.
QJsonObject jsonFromString( QString str ) {
    QJsonObject obj;

    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());

    // check validity of the document
    if(!doc.isNull())
    {
        if(doc.isObject())
            obj = doc.object();
        else
            qDebug() << "jsonFromString failed to parse Json: document is not JsonObject";
    }
    else
        qDebug() << "jsonFromString failed to parse Json: " << str;

    return obj;
}

// Will return 'null' if not found. So you can't check if value exist and null, or doesn't exist
QJsonValue readValueFromJson(const QJsonObject & jsonObj, QString path) {
    QStringList keys = path.split('.');

    QJsonValue result(jsonObj);

    for (auto key : keys) {
        if (result.isNull())
            return QJsonValue();

        if (!result.isObject() )
            return QJsonValue();

        result = result.toObject().value(key);
    }
    return result;
}

QString readStringFromJson(const QJsonObject & jsonObj, QString path, const QString * const defaultValue ) {
    QJsonValue result = readValueFromJson(jsonObj, path);
    if (result.isString())
        return result.toString();

    if (defaultValue== nullptr)
        return "";

    return *defaultValue;
}

//////////////////////////////////////////////// FILE TRANSACTIONS ///////////////////////////////////////

QPair<bool, QString> FileTransactionInfo::parseSlateFile( QString fn, FileTransactionType type ) {
    QString jsonStr = readTextFile(fn).join(' ');
    qDebug() << "parseTransaction for " << fn << " Body:" << jsonStr;

    fileName = fn;

    return parseSlateContent( jsonStr, type, "" );
}

QPair<bool, QString> FileTransactionInfo::parseSlateContent( QString slateContent, FileTransactionType type, QString slateSenderAddress ) {

    QJsonObject json = jsonFromString(slateContent);

    if (json.length()==0 ) {
        return QPair<bool, QString>(false, "Transaction content has wrong format. It is not an MWC slate.");
    }

    bool ok1 = true, ok2 = true, ok3 = true, ok4 = true;
    QString mwc = "mwc";
    QString coin = readStringFromJson(json, "coin_type", &mwc );
    if (coin != "mwc") {
        return QPair<bool, QString>(false, "The slate is not form MWC network.");
    }

    QString expectedNetwork = state::getStateContext()->wallet->getWalletConfig().getNetwork().toLower();
    QString network = readStringFromJson(json, "network_type", &expectedNetwork );
    if (network != expectedNetwork) {
        return QPair<bool, QString>(false, "The slate is form '"+network+"' network, expected is '"+expectedNetwork+"'.");
    }

    // V1
    int version = readValueFromJson(json, "version").toInt(-1);
    // V2
    if (version != 1) {
        QJsonObject version_info = readValueFromJson( json, "version_info" ).toObject();
        version = readValueFromJson(version_info, "version").toInt(-1);
    }
    // V0 doesn't have any version
    if (version<0)  // So any garbage assuming to be v0
        version = 0;

    // v0, v1 & v2
    transactionId = readStringFromJson( json, "id" );

    switch (version) {
        case 0:
        case 1:
            amount  = int64_t (readValueFromJson( json, "amount" ).toDouble(-10.0) + 0.5);
            fee     = int64_t (readValueFromJson( json, "fee" ).toDouble(-10.0) + 0.5);
            height  = readValueFromJson( json, "height" ).toInt(-1);
            lock_height = readValueFromJson( json, "lock_height" ).toInt(-1);
            break;
        case 2:
        case 3:
            amount  = readStringFromJson( json, "amount" ).toLongLong(&ok1);
            fee     = readStringFromJson( json, "fee" ).toLongLong(&ok2);
            height  = readStringFromJson( json, "height" ).toInt(&ok3);
            lock_height = readStringFromJson( json, "lock_height" ).toInt(&ok4);
            break;
        default:
            qDebug() << "Transaction file has unknown version " << version;
            return QPair<bool, QString>(false, "Content of the slate has unknown version. We unable to process this MWC slate.");
    }

    amount_fee_not_defined = false;
    bool compact_slate = readValueFromJson(json, "compact_slate").toBool(false);
    if (compact_slate && type == FileTransactionType::FINALIZE) {
        amount_fee_not_defined = true;
    }

    if (! (ok1 && ok2 && ok3 && ok4 && !transactionId.isEmpty() && ((amount>0 && fee>0) || amount_fee_not_defined) && height>0 && lock_height>=0) )
        return  QPair<bool, QString>(false, "Content of the slate is non complete MWC slate. Transaction details are not found.");

    // Looking for the receiver address
    if (!slateSenderAddress.isEmpty()) {
        fromAddress = slateSenderAddress;
    }
    else {
        // get it from the slate now
        switch (type) {
            case FileTransactionType::RECEIVE:
                fromAddress = readValueFromJson( json, "payment_proof.sender_address" ).toString();
                break;
            case FileTransactionType::FINALIZE:
                fromAddress = readValueFromJson( json, "payment_proof.receiver_address" ).toString();
                break;
            default:
                Q_ASSERT(false);
        }
    }

    // Same for v0, v1 & v2
    QJsonArray participant_data = readValueFromJson( json, "participant_data" ).toArray();
    int pdSz = participant_data.size();
    for (int i=0; i<pdSz; i++ ) {
         QJsonObject pdJson = participant_data[i].toObject();
         int id = readValueFromJson( pdJson, "id" ).toString("-1").toInt();
         if (id<0) // skipping noise
             continue;

         QString m = readStringFromJson( pdJson, "message" );
         if (! m.isEmpty()) {
             if (id==0)
                 senderMessage = m;
             else if (id==1) {
                 receiverMessage = m;
             }
             else {
                 Q_ASSERT(false);  // Expected only participants 0 & 1
             }
         }
    }

    // Verify the type of the transaction.
    // For now receive has 1 participant_data item and no 'part_sig' at participant_data
    //  Finalize mast have part_sig
    // participant_data with part_sig exist for all slate versions...
    if ( pdSz==0 ) {
        return QPair<bool, QString>(false, "Content of the slate is non complete MWC slate, 'participant_data' not found.");
    }

    bool hasPartSig = false;
    for (int i=0; i<pdSz; i++ ) {
        QString part_sig = readStringFromJson( participant_data[i].toObject(), "part_sig" );
        if (!part_sig.isEmpty()) {
            hasPartSig = true;
        }
    }

    switch (type) {
        case FileTransactionType::RECEIVE:
            if (pdSz>1 || hasPartSig) {
                return QPair<bool, QString>(false, "This slate was already received and need to be 'Finalized' now.");
            }
            break;
        case FileTransactionType::FINALIZE:
            if (!hasPartSig) {
                return QPair<bool, QString>(false, "This slate need to be signed with 'Receive' operation first.");
            }
            break;
        default:
            Q_ASSERT(false);
    }

    return  QPair<bool, QString>(true, "");
}


}

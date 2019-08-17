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

bool FileTransactionInfo::parseTransaction( QString fn ) {
    QString jsonStr = readTextFile(fn).join(' ');

    QJsonObject json = jsonFromString(jsonStr);

    bool ok1 = true, ok2 = true, ok3 = true, ok4 = true;

    fileName = fn;
    transactionId = readStringFromJson( json, "id" );
    amount  = readStringFromJson( json, "amount" ).toLongLong(&ok1);
    fee     = readStringFromJson( json, "fee" ).toLongLong(&ok2);
    height  = readStringFromJson( json, "height" ).toInt(&ok3);
    lock_height = readStringFromJson( json, "lock_height" ).toInt(&ok4);

    QJsonArray participant_data = readValueFromJson( json, "participant_data" ).toArray();
    for (int i=0; i<participant_data.size(); i++ ) {
         QString m = readStringFromJson( participant_data[i].toObject(), "message" );
         if (! m.isEmpty()) {
             if (!message.isEmpty())
                 message += "; ";
             message += m;
         }

    }

    return ok1 && ok2 && ok3 && ok4 && !transactionId.isEmpty() && amount >0 && fee>0 && height>0 && lock_height>0;
}


}

// Copyright 2021 The MWC Developers
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

#include "TaskSwapMkt.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include "../mwc713.h"
#include "utils.h"
#include "../../core/Notification.h"

namespace wallet {


////////////////////////////////////////////////////////////////////////////////
// TaskSwapMktNewMessage

bool TaskSwapMktNewMessage::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size() == 1);

    const WEvent &evt = events[0];

    if (evt.event == S_MKT_ACCEPT_OFFER || evt.event == S_MKT_FAIL_BIDDING) {
        QStringList prms = evt.message.split('|');
        if (prms.size() != 2)
            return false;

        QString wallet_tor_address = prms[0];
        QString offer_id = prms[1];

        wallet713->notifyAboutNewMktMessage(evt.event, wallet_tor_address, offer_id);
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// TaskCreateIntegrityFee

bool TaskCreateIntegrityFee::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // RES: {"create_res":[{"ask_fee":"10000000","conf":true,"expiration_height":758705,"fee":"20000000","uuid":"1171b5ec-0c64-467c-81e0-fbbfd7e38c33"}]}
    QVector<wallet::IntegrityFees> resFees;

    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setCreateIntegrityFee( "Unable to parse mwc713 output", resFees);
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isObject());

            QJsonArray arr = jsonDoc.object().value("create_res").toArray();
            if (arr.isEmpty()) {
                wallet713->setCreateIntegrityFee( "Unable to parse mwc713 output", resFees);
                return true;
            }

            for (int i = 0; i < arr.size(); i++) {
                QJsonValue val = arr.at(i);
                if (!val.isObject()) {
                    wallet713->setCreateIntegrityFee( "Unable to parse mwc713 output", resFees);
                    return true;
                }
                QJsonObject feeJson = val.toObject();
                resFees.push_back(
                        wallet::IntegrityFees(feeJson["conf"].toBool(),
                            feeJson["expiration_height"].toInt(),
                            feeJson["ask_fee"].toString().toLongLong(),
                            feeJson["fee"].toString().toLongLong(),
                            feeJson["uuid"].toString())
                );
            }
            // Success case
            wallet713->setCreateIntegrityFee("", resFees);
            return true;
        }
    }

    wallet713->setCreateIntegrityFee(getErrorMessage(events,"Unable to read CreateIntegrityFee data"), resFees);
    return true;
}

QString TaskCreateIntegrityFee::generateCommandLine(const QString &account, double mwcReserve, const QVector<double> &fees) const {
    Q_ASSERT(mwcReserve>0.0);
    Q_ASSERT(!fees.isEmpty());
    Q_ASSERT(!account.isEmpty());
    QString feesStr;
    for (auto & f : fees) {
        if (!feesStr.isEmpty())
            feesStr += ",";
        feesStr += QString::number(f);
    }

    return "integrity --account " + util::toMwc713input(account) + " --reserve " + QString::number(mwcReserve) + "  --create " + feesStr + " --json";
}

//////////////////////////////////////////////////////////////////////////////////////
//  TaskRequestIntegrityFee

bool TaskRequestIntegrityFee::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // JSON: {"balance":"885000000","tx_fee":[{"conf":true,"expiration_height":760163,"fee":"10000000","uuid":"0a961185-e1bb-4b8a-bf23-ea15de966ba9"}]}
    QVector<wallet::IntegrityFees> fees;

    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setRequestIntegrityFees("Unable to parse mwc713 output", 0, fees);
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isObject());

            QJsonObject response = jsonDoc.object();

            QJsonArray arr = response.value("tx_fee").toArray();
            for (int i = 0; i < arr.size(); i++) {
                QJsonValue val = arr.at(i);
                if (!val.isObject()) {
                    wallet713->setRequestIntegrityFees("Unable to parse mwc713 output", 0, fees);
                    return true;
                }
                QJsonObject feeJson = val.toObject();
                int64_t fee = feeJson["fee"].toString().toLongLong();
                fees.push_back(
                        wallet::IntegrityFees(feeJson["conf"].toBool(),
                                              feeJson["expiration_height"].toInt(),
                                              fee,
                                              fee,
                                              feeJson["uuid"].toString())
                );
            }
            // Success case
            wallet713->setRequestIntegrityFees("", response["balance"].toString().toLongLong(), fees);
            return true;
        }
    }

    wallet713->setRequestIntegrityFees(getErrorMessage(events,"Unable to read RequestIntegrityFee data"), 0, fees);
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//  TaskWithdrawIntegrityFees

bool TaskWithdrawIntegrityFees::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    for (auto &ln : lns) {
        // 0.881 MWC was transferred to account default
        const QString okStr(" MWC was transferred to account ");
        const QString & l = ln.message;

        if (l == "There are no integrity funds to withdraw") {
            wallet713->setWithdrawIntegrityFees( "There are no reserved integrity funds.", 0.0, "" );
            return true;
        }

        int idx = l.indexOf(okStr);
        if (idx>0) {
            bool ok = false;
            double mwc = l.left(idx).toDouble(&ok);
            if (ok) {
                QString account = l.right( l.length() - okStr.length() - idx );
                wallet713->setWithdrawIntegrityFees( "", mwc, account );
                return true;
            }
        }
    }

    wallet713->setWithdrawIntegrityFees( getErrorMessage(events, "Unable to process Withdraw Integrity Fee response"), 0.0, "" );
    return true;
}

//////////////////////////////////////////////////////////////////////
// TaskRequestMessagingStatus

bool TaskRequestMessagingStatus::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // JSON: {"broadcasting":[{"broadcasting_interval":60,"fee":"10000000","message":"{}","published_time":49,"uuid":"7f0a6a89-5ad5-40cb-b204-0805ffcd1903"}],"gossippub_peers":null,"received_messages":0,"topics":["swapmarketplace","testing"]}
    wallet::MessagingStatus emptyStatus;

    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setRequestMessagingStatus("Unable to parse mwc713 output", emptyStatus);
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isObject());

            QJsonObject response = jsonDoc.object();

            wallet::MessagingStatus status(response);
            // Success case
            wallet713->setRequestMessagingStatus("", wallet::MessagingStatus(response));
            return true;
        }
    }

    wallet713->setRequestMessagingStatus( getErrorMessage(events, "Unable to process Messaging Status response"), emptyStatus );
    return true;
}

/////////////////////////////////////////////////////////////////////////
// TaskMessagingPublish

bool TaskMessagingPublish::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // The messages is published. ID 7226968b-ad98-4381-96df-9a9d3f30c97d
    const QString prefixStr("The messages is published. ID ");

    for ( auto &ln : lns ) {
        if ( ln.message.startsWith(prefixStr) ) {
            QString msgUuid = ln.message.mid( prefixStr.length() );
            wallet713->setMessagingPublish(id, msgUuid, "");
            return true;
        }
    }

    wallet713->setMessagingPublish(id, "", getErrorMessage(events, "Unable to process publish message response") );
    return true;
}

///////////////////////////////////////////////////////////////
// TaskCheckIntegrity

bool TaskCheckIntegrity::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // JSON: {"expired_msgs":[]}
    // Content
    // { "uuid": msg.uuid.to_string(),
    //            "topic": msg.topic.to_string(),
    //            "message": msg.message
    //}
    wallet::MessagingStatus emptyStatus;

    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setCheckIntegrity("Unable to read Integrity Check output", {});
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isObject());

            QJsonObject response = jsonDoc.object();
            QJsonArray expired_msgs = response["expired_msgs"].toArray();
            QVector<QString> msgUuid;
            for ( int i=0; i<expired_msgs.size(); i++ ) {
                QJsonObject msg = expired_msgs[i].toObject();
                QString uuid = msg["uuid"].toString();
                if (!uuid.isEmpty())
                    msgUuid.push_back(uuid);
            }
            wallet713->setCheckIntegrity("", msgUuid);
        }
    }

    wallet713->setCheckIntegrity(getErrorMessage(events, "Unable to process Integrity Check response"), {});
    return true;
}

///////////////////////////////////////////////////////////////////////
// TaskMessageWithdraw

bool TaskMessageWithdraw::processTask(const QVector<WEvent> &events ) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // JSON: {"remove_message":"bb31f6c1-c717-40c3-adbc-2c30d68a39ee"}
    // JSON: {"remove_message":null}
    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setMessageWithdraw("", "Unable to read Withdraw Message output");
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isObject());
            QJsonObject response = jsonDoc.object();

            wallet713->setMessageWithdraw( response["remove_message"].toString(), "");
            return true;
        }
    }

    wallet713->setMessageWithdraw("", getErrorMessage(events, "Unable to process Withdraw Message response") );
    return true;
}

//////////////////////////////////////////////////////////////////////
//  TaskRequestReceiveMessages

bool TaskRequestReceiveMessages::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // JSON: {"receive_messages":[]}
    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setReceiveMessages("Unable to read request receive messages output", {});
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isObject());
            QJsonObject response = jsonDoc.object();

            QJsonArray messages = response["receive_messages"].toArray();
            QVector<ReceivedMessages> res;

            for (int i=0; i<messages.size(); i++) {
                QJsonObject msg = messages[i].toObject();

                res.push_back(ReceivedMessages(msg["topic"].toString(),
                                 msg["fee"].toString().toLongLong(),
                                 msg["message"].toString(),
                                 msg["wallet"].toString()));
            }

            wallet713->setReceiveMessages( "", res );
            return true;
        }
    }

    wallet713->setReceiveMessages(getErrorMessage(events, "Unable to process request receive messages response"), {} );
    return true;
}

//////////////////////////////////////////////////////////////////////
//  TaskStartListenOnTopic

bool TaskStartListenOnTopic::processTask(const QVector<WEvent> &events) {
    wallet713->setStartListenOnTopic(getErrorMessage(events, "") );
    return true;
}

//////////////////////////////////////////////////////////////////////
//  TaskStopListenOnTopic

bool TaskStopListenOnTopic::processTask(const QVector<WEvent> &events) {
    wallet713->setStopListenOnTopic(getErrorMessage(events, "") );
    return true;
}

//////////////////////////////////////////////////////////////////////
//  TasksSendMarketplaceMessage

bool TasksSendMarketplaceMessage::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // JSON: {"receive_messages":[]}
    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();
            wallet713->setSendMarketplaceMessage("", jsonStr, offer_id, wallet_tor_address);
            return true;
        }
    }

    wallet713->setSendMarketplaceMessage(getErrorMessage(events, "Unable to process SendMarketplaceMessage response"), "", offer_id, wallet_tor_address );
    return true;

}



}



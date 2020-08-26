// Copyright 2020 The MWC Developers
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

#include "TaskSwap.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include "../mwc713.h"

namespace wallet {

static QString getErrorMessage(const QVector<WEvent> &events, QString defaultMessage) {
    QString errors;
    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR );
    for (auto & e : errs ) {
        if (errors.length()>0)
            errors += "; ";
        errors += e.message;
    }
    if (errors.isEmpty()) {
        errors = defaultMessage;
    }
    return errors;
}

// ---------------- TaskSwapNewTradeArrive ----------------

bool TaskSwapNewTradeArrive::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size() == 1);

    const WEvent &evt = events[0];

    if (evt.event == S_SWAP_GET_OFFER) {
        QStringList prms = evt.message.split('|');
        if (prms.size() != 2)
            return false;

        QString currency = prms[0];
        QString swapId = prms[1];

        wallet713->notifyAboutNewSwapTrade(currency, swapId);
        return true;
    }
    return false;
}

// ---------------- TaskGetSwapTrades ----------------------
bool TaskGetSwapTrades::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    for ( auto & ln : lns ) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid( strlen("JSON: ") ).trimmed();

            QJsonParseError error;
            QJsonDocument   jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            QVector<wallet::SwapInfo> swapTrades;

            if (error.error != QJsonParseError::NoError || !jsonDoc.isArray()) {
                wallet713->setRequestSwapTrades( swapTrades, "Unable to parse mwc713 output" );
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT( error.error == QJsonParseError::NoError );
            Q_ASSERT(jsonDoc.isArray());

            QJsonArray arr =  jsonDoc.array();
            for ( int i=0; i<arr.size(); i++ ) {
                QJsonValue val =arr.at(i);
                if (!val.isObject()) {
                    wallet713->setRequestSwapTrades( swapTrades, "Unable to parse mwc713 output" );
                    return true;
                }
                QJsonObject swapInfoJson = val.toObject();
                wallet::SwapInfo swap_info;
                swap_info.setData( swapInfoJson["info"].toString(),
                                   swapInfoJson["swap_id"].toString(),
                                   swapInfoJson["start_time"].toString().toLongLong(),
                                   swapInfoJson["state"].toString(),
                                   swapInfoJson["action"].toString(),
                                   swapInfoJson["expiration"].toString().toLongLong(),
                                   swapInfoJson["is_seller"].toBool(),
                                   swapInfoJson["secondary_address"].toString()
                                   );

                swapTrades.push_back(swap_info);
            }
            // Let's sort them by time
            std::sort(swapTrades.begin(), swapTrades.end(), [](const wallet::SwapInfo &s1, const wallet::SwapInfo &s2) { return s1.startTime > s2.startTime; } );

            // Success case
            wallet713->setRequestSwapTrades( swapTrades, "" );
            return true;
        }
    }

    wallet713->setRequestSwapTrades( {}, getErrorMessage(events, "Unable to read swap list data") );
    return true;
}

// --------------------- TaskDeleteSwapTrades -------------------------
bool TaskDeleteSwapTrade::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    for (auto &ln : lns) {
        if (ln.message.contains("was sucessfully deleted")) {
            wallet713->setDeleteSwapTrade(swapId, "");
            return true;
        }
    }

    wallet713->setDeleteSwapTrade( swapId, getErrorMessage(events, "Unable to read delete trade data") );
    return true;
}

// ------------------------- TaskCreateNewSwapTrade ---------------------
bool TaskCreateNewSwapTrade::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    for (auto &ln : lns) {
        if (ln.message.startsWith("New Swap Trade created:")) {
            QString swapId = ln.message.mid( strlen("New Swap Trade created:") ).trimmed();
            wallet713->setCreateNewSwapTrade(swapId, "");
            return true;
        }
    }

    wallet713->setCreateNewSwapTrade( "", getErrorMessage(events, "Unable to create a new Swap Trade") );
    return true;
}


// --------------------- TaskCancelSwapTrades -------------------------
bool TaskCancelSwapTrade::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    for (auto &ln : lns) {
        if (ln.message.contains("was successfully adjusted")) {
            wallet713->setCancelSwapTrade(swapId, "");
            return true;
        }
    }

    wallet713->setCancelSwapTrade( swapId, getErrorMessage(events, "Unable to cancel the Swap Trade " + swapId) );
    return true;
}

// ------------------ TaskTradeDetails -------------------
bool TaskTradeDetails::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    wallet::SwapTradeInfo swap;
    swap.swapId = swapId;

    QVector<SwapExecutionPlanRecord> executionPlan;
    QVector<SwapJournalMessage> tradeJournal;

    for ( auto & ln : lns ) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid( strlen("JSON: ") ).trimmed();

            QJsonParseError error;
            QJsonDocument   jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setRequestTradeDetails( swap, executionPlan, "", tradeJournal, "Unable to parse mwc713 output" );
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT( error.error == QJsonParseError::NoError );
            Q_ASSERT(jsonDoc.isObject());

            QJsonObject swapObj =  jsonDoc.object();

            swap.setData( swapObj["swapId"].toString(), swapObj["isSeller"].toBool(), swapObj["mwcAmount"].toString().toDouble(),
                          swapObj["secondaryAmount"].toString().toDouble(),
                          swapObj["secondaryCurrency"].toString(),  swapObj["secondaryAddress"].toString(), swapObj["secondaryFee"].toString().toDouble(),
                          swapObj["secondaryFeeUnits"].toString(), swapObj["mwcConfirmations"].toInt(), swapObj["secondaryConfirmations"].toInt(),
                          swapObj["messageExchangeTimeLimit"].toInt(), swapObj["redeemTimeLimit"].toInt(), swapObj["sellerLockingFirst"].toBool(),
                          swapObj["mwcLockHeight"].toInt(), swapObj["mwcLockTime"].toString().toLongLong(), swapObj["secondaryLockTime"].toString().toLongLong(),
                          swapObj["communicationMethod"].toString(), swapObj["communicationAddress"].toString() );

            QJsonArray execPlan = swapObj["roadmap"].toArray();
            for (int i=0; i<execPlan.size(); i++) {
                QJsonObject planItm = execPlan.at(i).toObject();
                SwapExecutionPlanRecord planRecord;
                planRecord.setData( planItm["active"].toBool(), planItm["end_time"].toString().toLongLong(), planItm["name"].toString() );
                executionPlan.push_back(planRecord);
            }

            QString currentAction = swapObj["currentAction"].toString();
            if (currentAction=="None")
                currentAction="";

            QJsonArray journal = swapObj["journal_records"].toArray();
            for (int i=0; i<journal.size(); i++) {
                QJsonObject jrnl = journal.at(i).toObject();
                SwapJournalMessage msg;
                msg.setData( jrnl["message"].toString(), jrnl["time"].toString().toLongLong() );
                tradeJournal.push_back(msg);
            }

            // Success case
            wallet713->setRequestTradeDetails(swap, executionPlan, currentAction, tradeJournal,  "" );
            return true;
        }
    }

    wallet713->setRequestTradeDetails( swap, executionPlan, "", tradeJournal, getErrorMessage(events, "Unable to read swap list data") );
    return true;
}

// --------------- TaskAdjustTrade -------------------
bool TaskAdjustTrade::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    for (auto &ln : lns) {
        if (ln.message.contains("was successfully adjusted")) {
            wallet713->setAdjustSwapData(swapId, adjustCommand, "");
            return true;
        }
    }

    wallet713->setAdjustSwapData( swapId, adjustCommand, getErrorMessage(events, "Unable update the Swap Trade " + swapId) );
    return true;
}

QString TaskAdjustTrade::generateCommandLine(const QString & swapId, const QString & adjustCmd, const QString & param1, const QString & param2) const {
    if (adjustCmd=="destination") {
        return "swap --adjust destination  --method " + param1 + " --dest " + param2 + " -i " + swapId;
    }
    else if (adjustCmd=="secondary_address") {
        return "swap --adjust secondary_address --secondary_address " + param1 + " -i " + swapId;
    }
    else if (adjustCmd=="secondary_fee") {
        return "swap --adjust secondary_fee --secondary_fee " + param1 + " -i " + swapId;
    }
    else {
        Q_ASSERT(false);
        return "";
    }

}

// --------------- TaskPerformAutoSwapStep -----------------
bool TaskPerformAutoSwapStep::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    QVector<SwapExecutionPlanRecord> executionPlan;
    QVector<SwapJournalMessage> tradeJournal;

    for ( auto & ln : lns ) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid( strlen("JSON: ") ).trimmed();

            QJsonParseError error;
            QJsonDocument   jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setPerformAutoSwapStep(swapId, false, "", "",
                                    executionPlan, tradeJournal, "Unable to parse mwc713 output for autoswap trade " + swapId );
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT( error.error == QJsonParseError::NoError );
            Q_ASSERT(jsonDoc.isObject());

            QJsonObject swapObj =  jsonDoc.object();

            QJsonArray execPlan = swapObj["roadmap"].toArray();
            for (int i=0; i<execPlan.size(); i++) {
                QJsonObject planItm = execPlan.at(i).toObject();
                SwapExecutionPlanRecord planRecord;
                planRecord.setData( planItm["active"].toBool(), planItm["end_time"].toString().toLongLong(), planItm["name"].toString() );
                executionPlan.push_back(planRecord);
            }

            QString currentAction = swapObj["currentAction"].toString();
            if (currentAction=="None")
                currentAction="";

            QJsonArray journal = swapObj["journal_records"].toArray();
            for (int i=0; i<journal.size(); i++) {
                QJsonObject jrnl = journal.at(i).toObject();
                SwapJournalMessage msg;
                msg.setData( jrnl["message"].toString(), jrnl["time"].toString().toLongLong() );
                tradeJournal.push_back(msg);
            }

            // Success case
            wallet713->setPerformAutoSwapStep(swapId,
                    swapObj["autowsap_done"].toBool(),
                    currentAction,
                    swapObj["currentState"].toString(),
                    executionPlan,
                    tradeJournal,
                    "" );
            return true;
        }
    }

    wallet713->setPerformAutoSwapStep(swapId, false, "", "",
                                      executionPlan, tradeJournal, "Unable to read output for autoswap trade " + swapId );
    return true;
}


}

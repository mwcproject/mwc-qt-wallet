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
#include "utils.h"
#include "../../core/Notification.h"

namespace wallet {

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

    if (evt.event == S_SWAP_GET_MESSAGE) {
        QStringList prms = evt.message.split('|');
        if (prms.size() != 1)
            return false;

        QString swapId = prms[0];

        wallet713->notifyAboutSwapMessage(swapId);
        return true;
    }
    return false;
}

// ---------------- TaskGetSwapTrades ----------------------
bool TaskGetSwapTrades::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            QVector<wallet::SwapInfo> swapTrades;

            if (error.error != QJsonParseError::NoError || !jsonDoc.isArray()) {
                wallet713->setRequestSwapTrades(cookie, swapTrades, "Unable to parse mwc713 output");
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isArray());

            QJsonArray arr = jsonDoc.array();
            for (int i = 0; i < arr.size(); i++) {
                QJsonValue val = arr.at(i);
                if (!val.isObject()) {
                    wallet713->setRequestSwapTrades(cookie, swapTrades, "Unable to parse mwc713 output");
                    return true;
                }
                QJsonObject swapInfoJson = val.toObject();
                wallet::SwapInfo swap_info;
                swap_info.setData(
                        swapInfoJson["mwc_amount"].toString(),
                        swapInfoJson["secondary_amount"].toString(),
                        swapInfoJson["secondary_currency"].toString(),
                        swapInfoJson["swap_id"].toString(),
                        swapInfoJson["tag"].toString(),
                        swapInfoJson["start_time"].toString().toLongLong(),
                        swapInfoJson["state_cmd"].toString(),
                        swapInfoJson["state"].toString(),
                        swapInfoJson["action"].toString(),
                        swapInfoJson["expiration"].toString().toLongLong(),
                        swapInfoJson["is_seller"].toBool(),
                        swapInfoJson["secondary_address"].toString(),
                        swapInfoJson["last_process_error"].toString()
                  );

                swapTrades.push_back(swap_info);
            }
            // Let's sort them by time
            std::sort(swapTrades.begin(), swapTrades.end(), [](const wallet::SwapInfo &s1, const wallet::SwapInfo &s2) {
                return s1.startTime > s2.startTime;
            });

            // Success case
            wallet713->setRequestSwapTrades(cookie, swapTrades, "");
            return true;
        }
    }

    wallet713->setRequestSwapTrades(cookie, {}, getErrorMessage(events, "Unable to read swap list data"));
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

    wallet713->setDeleteSwapTrade(swapId, getErrorMessage(events, "Unable to read delete trade data"));
    return true;
}

// ------------------------- TaskCreateNewSwapTrade ---------------------

QString TaskCreateNewSwapTrade::generateCommandLine(
                            QVector<QString> outputs, // If defined, those outputs will be used to trade. They might belong to another trade, that if be fine.
                            int min_confirmations,
                            QString mwcAmount, QString  secAmount, QString secondary,
                            QString redeemAddress,
                            double secTxFee,
                            bool sellerLockFirst,
                            int messageExchangeTimeMinutes,
                            int redeemTimeMinutes,
                            int mwcConfirmationNumber,
                            int secondaryConfirmationNumber,
                            QString communicationMethod,
                            QString communicationAddress,
                            QString electrum_uri1,
                            QString electrum_uri2,
                            QString mkt_trade_tag,
                            bool dryRun) const
{
    Q_ASSERT(messageExchangeTimeMinutes>0);
    Q_ASSERT(redeemTimeMinutes>0);
    Q_ASSERT(mwcConfirmationNumber>0);
    Q_ASSERT(secondaryConfirmationNumber>0);

    QString cmdLine = "swap_start --message_exchange_time " + QString::number(messageExchangeTimeMinutes) +
            " --mwc_confirmations " + QString::number(mwcConfirmationNumber) +
            " --secondary_confirmations " + QString::number(secondaryConfirmationNumber) +
            " --redeem_time " + QString::number(redeemTimeMinutes) +
            " --secondary_address " + redeemAddress +
            " --secondary_fee " + QString::number(secTxFee) +
            " --secondary_amount " + secAmount +
            " --secondary_currency " + secondary +
            " --who_lock_first " + (sellerLockFirst ? "seller" : "buyer");

    if (!outputs.isEmpty()) {
        cmdLine += " --outputs " + QStringList(outputs.toList()).join(",");
    }

    if (mwcAmount.isEmpty())
        cmdLine += " --mwc_amount 0.0";
    else
        cmdLine += " --mwc_amount " + mwcAmount;

    if (!communicationMethod.isEmpty())
        cmdLine += " --method " + communicationMethod;
    else
        cmdLine += " --method file";

    if (!communicationAddress.isEmpty())
        cmdLine += " --dest " + communicationAddress;
    else
        cmdLine += " --dest del.me";

    if (min_confirmations>0)
        cmdLine += " --min_conf " + QString::number(min_confirmations);

    if (!electrum_uri1.isEmpty())
        cmdLine += " --electrum_uri1 " + electrum_uri1;
    if (!electrum_uri2.isEmpty())
        cmdLine += " --electrum_uri2 " + electrum_uri2;

    if (dryRun)
        cmdLine += " --dry_run";

    if (!mkt_trade_tag.isEmpty())
        cmdLine += " --tag " + mkt_trade_tag;

    return cmdLine;
}


bool TaskCreateNewSwapTrade::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    for (auto &ln : lns) {
        if (ln.message.startsWith("New Swap Trade created:")) {
            QString swapId = ln.message.mid(strlen("New Swap Trade created:")).trimmed();
            wallet713->setCreateNewSwapTrade( tag, dryRun, params, swapId, "");
            return true;
        }
    }

    wallet713->setCreateNewSwapTrade(tag, dryRun, params, "", getErrorMessage(events, "Unable to create a new Swap Trade"));
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

    wallet713->setCancelSwapTrade(swapId, getErrorMessage(events, "Unable to cancel the Swap Trade " + swapId));
    return true;
}

// ------------------ TaskTradeDetails -------------------
bool TaskTradeDetails::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    wallet::SwapTradeInfo swap;
    swap.swapId = swapId;

    QVector<SwapExecutionPlanRecord> executionPlan;
    QVector<SwapJournalMessage> tradeJournal;

    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setRequestTradeDetails(swap, executionPlan, "", tradeJournal,
                                                  "Unable to parse mwc713 output");
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isObject());

            QJsonObject swapObj = jsonDoc.object();

            QString feeUnits = swapObj["secondaryFeeUnits"].toArray().first().toString();

            swap.setData(swapObj["swapId"].toString(),
                         swapObj["tag"].toString(),
                         swapObj["isSeller"].toBool(),
                         swapObj["mwcAmount"].toString().toDouble(),
                         swapObj["secondaryAmount"].toString().toDouble(),
                         swapObj["secondaryCurrency"].toString(), swapObj["secondaryAddress"].toString(),
                         swapObj["secondaryFee"].toString().toDouble(),
                         feeUnits,
                         swapObj["mwcConfirmations"].toInt(),
                         swapObj["secondaryConfirmations"].toInt(),
                         swapObj["messageExchangeTimeLimit"].toInt(), swapObj["redeemTimeLimit"].toInt(),
                         swapObj["sellerLockingFirst"].toBool(),
                         swapObj["mwcLockHeight"].toInt(), swapObj["mwcLockTime"].toString().toLongLong(),
                         swapObj["secondaryLockTime"].toString().toLongLong(),
                         swapObj["communicationMethod"].toString(), swapObj["communicationAddress"].toString(),
                         swapObj["electrumNodeUri1"].toString());

            QJsonArray execPlan = swapObj["roadmap"].toArray();
            for (int i = 0; i < execPlan.size(); i++) {
                QJsonObject planItm = execPlan.at(i).toObject();
                SwapExecutionPlanRecord planRecord;
                planRecord.setData(planItm["active"].toBool(), planItm["end_time"].toString().toLongLong(),
                                   planItm["name"].toString());
                executionPlan.push_back(planRecord);
            }

            QString currentAction = swapObj["currentAction"].toString();
            if (currentAction == "None")
                currentAction = "";

            QJsonArray journal = swapObj["journal_records"].toArray();
            for (int i = 0; i < journal.size(); i++) {
                QJsonObject jrnl = journal.at(i).toObject();
                SwapJournalMessage msg;
                msg.setData(jrnl["message"].toString(), jrnl["time"].toString().toLongLong());
                tradeJournal.push_back(msg);
            }

            // Success case
            wallet713->setRequestTradeDetails(swap, executionPlan, currentAction, tradeJournal, "");
            return true;
        }
    }

    wallet713->setRequestTradeDetails(swap, executionPlan, "", tradeJournal,
                                      getErrorMessage(events, "Unable to read swap list data"));
    return true;
}

// --------------- TaskAdjustTrade -------------------
bool TaskAdjustTrade::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    for (auto &ln : lns) {
        if (ln.message.contains("was successfully adjusted")) {
            wallet713->setAdjustSwapData(swapId, call_tag, "");
            return true;
        }
    }

    wallet713->setAdjustSwapData(swapId, call_tag,
                                 getErrorMessage(events, "Unable update the Swap Trade " + swapId));
    return true;
}

QString TaskAdjustTrade::generateCommandLine(const QString &swapId,
                                             const QString &destinationMethod, const QString & destinationDest,
                                             const QString &secondaryAddress,
                                             const QString &secondaryFee,
                                             const QString &electrumUri1,
                                             const QString &tag) const {

    QStringList adjustCmd;

    QString params;

    if (!destinationDest.isEmpty() && !destinationDest.isEmpty()) {
        adjustCmd.push_back("destination");
        params += " --method " + destinationMethod + " --dest " + destinationDest;
    }

    if (!secondaryAddress.isEmpty()) {
        adjustCmd.push_back("secondary_address");
        params += " --secondary_address " + secondaryAddress;
    }

    if (!secondaryFee.isEmpty()) {
        adjustCmd.push_back("secondary_fee");
        params += " --secondary_fee " + secondaryFee;
    }

    if (!electrumUri1.isEmpty()) {
        adjustCmd.push_back("electrumx_uri");
        params += " --electrum_uri1 \"" + electrumUri1 + "\"";
    }

    if (!tag.isEmpty()) {
        adjustCmd.push_back("tag");
        params += " --tag \"" + tag + "\"";
    }

    Q_ASSERT(!adjustCmd.isEmpty());
    return "swap --adjust " + adjustCmd.join(",") + params + " -i " + swapId;
}

// --------------- TaskPerformAutoSwapStep -----------------
bool TaskPerformAutoSwapStep::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    QVector<SwapExecutionPlanRecord> executionPlan;
    QVector<SwapJournalMessage> tradeJournal;

    for (auto &ln : lns) {
        if (ln.message.startsWith("JSON: ")) {
            QString jsonStr = ln.message.mid(strlen("JSON: ")).trimmed();

            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
                wallet713->setPerformAutoSwapStep(swapId, "", "", "", "",
                                                  executionPlan, tradeJournal,
                                                  "Unable to parse mwc713 output for autoswap trade " + swapId);
                return true;
            }

            // Internal data, no error expected
            Q_ASSERT(error.error == QJsonParseError::NoError);
            Q_ASSERT(jsonDoc.isObject());

            QJsonObject swapObj = jsonDoc.object();

            QJsonArray execPlan = swapObj["roadmap"].toArray();
            for (int i = 0; i < execPlan.size(); i++) {
                QJsonObject planItm = execPlan.at(i).toObject();
                SwapExecutionPlanRecord planRecord;
                planRecord.setData(planItm["active"].toBool(), planItm["end_time"].toString().toLongLong(),
                                   planItm["name"].toString());
                executionPlan.push_back(planRecord);
            }

            QString currentAction = swapObj["currentAction"].toString();
            if (currentAction == "None")
                currentAction = "";

            QJsonArray journal = swapObj["journal_records"].toArray();
            for (int i = 0; i < journal.size(); i++) {
                QJsonObject jrnl = journal.at(i).toObject();
                SwapJournalMessage msg;
                msg.setData(jrnl["message"].toString(), jrnl["time"].toString().toLongLong());
                tradeJournal.push_back(msg);
            }

            // Success case
            wallet713->setPerformAutoSwapStep(swapId,
                                              swapObj["stateCmd"].toString(),
                                              currentAction,
                                              swapObj["currentState"].toString(),
                                              swapObj["last_process_error"].toString(),
                                              executionPlan,
                                              tradeJournal,
                                              "");

            return true;
        }
    }

    wallet713->setPerformAutoSwapStep(swapId, "", "", "","",
                                      executionPlan, tradeJournal,
                                      getErrorMessage(events, "Unable to read output for autoswap trade " + swapId));
    return true;
}

// ------------------------ TaskBackupSwapTradeData -----------------------------------
bool TaskBackupSwapTradeData::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);
    for (auto &ln : lns) {
        if (ln.message.startsWith("Swap trade is exported to ")) {
            QString fn = ln.message.mid(strlen("Swap trade is exported to ")).trimmed();
            wallet713->setBackupSwapTradeData(swapId, fn, "");
            return true;
        }
    }

    // We failed, must be some errors
    wallet713->setBackupSwapTradeData(swapId, "",
                                      getErrorMessage(events, "Unable to export data for Swap Trade " + swapId));
    return true;
}

// ------------------------- TaskRestoreSwapTradeData ----------------------------
bool TaskRestoreSwapTradeData::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    // Swap trade 6f690448-3b89-47cf-9b72-5738e8d32344 is restored from the file /tmp/trade2.swap
    for (auto &ln : lns) {
        const QString & msg = ln.message;
        if (msg.startsWith("Swap trade")  && msg.contains("is restored from the file") ) {
            int idx1 = strlen("Swap trade ");
            int idx2 = msg.indexOf(' ', idx1+1);

            int idx3 = msg.indexOf(" file ") + strlen(" file ");

            if (idx1<idx2 && idx2<idx3) {
                QString tradeId = msg.mid(idx1, idx2-idx1).trimmed();
                QString fn = msg.mid(idx3).trimmed();
                wallet713->setRestoreSwapTradeData(tradeId, fn, "");
                return true;
            }
        }
    }

    // We failed, must be some errors
    wallet713->setRestoreSwapTradeData("", fileName,
                                      getErrorMessage(events, "Unable to restore the trade form the file " + fileName));
    return true;
}

// ------------------------- TaskAdjustTradeState ----------------------------
bool TaskAdjustTradeState::processTask(const QVector<WEvent> &events) {
    QVector<WEvent> lns = filterEvents(events, WALLET_EVENTS::S_LINE);

    for (const auto & l : lns) {
        if (l.message.contains("was successfully adjusted")) {
            // We are good.
            // Swap trade b87f00f4-381b-4f38-aec2-972871f47c76 was successfully adjusted. New state: Post Refund Transaction
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, l.message );
            return true;
        }
    }

    // Probably error happens. Should be reported automatically
    return true;
}


}

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

#ifndef MWC_QT_WALLET_TASKSWAP_H
#define MWC_QT_WALLET_TASKSWAP_H

#include "../mwc713task.h"
#include "../../util/stringutils.h"

namespace wallet {


// It is listener task. No input can be defined.
// Listening for a new Trade to arrive
class TaskSwapNewTradeArrive : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3600*1000*5; // NA in any case

    TaskSwapNewTradeArrive( MWC713 *wallet713 ) :
            Mwc713Task("TaskSwapNewTradeArrive", "", wallet713,"") {}

    virtual ~TaskSwapNewTradeArrive() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
};

// Get list of the trades
class TaskGetSwapTrades : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*40; // 40 seconds can be possible because of the status update and connection errors

    TaskGetSwapTrades( MWC713 *wallet713 ) :
                Mwc713Task("TaskSwapTrades",
                "swap --list --check --json_format",
                wallet713, "") {}

    virtual ~TaskGetSwapTrades() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};

// Delete single trade.
class TaskDeleteSwapTrade : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*2;

    TaskDeleteSwapTrade( MWC713 *wallet713, QString _swapId ) :
            Mwc713Task("TaskDeleteSwapTrades",
                       "swap --remove -i " + _swapId,
                       wallet713, ""),
                       swapId(_swapId) {}

    virtual ~TaskDeleteSwapTrade() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString swapId;
};

// Create new swap trade
class TaskCreateNewSwapTrade : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*15;

    TaskCreateNewSwapTrade( MWC713 *wallet713,
                            int min_confirmations,
                            double mwc, double btc, QString secondary,
                            QString redeemAddress,
                            bool sellerLockFirst,
                            int messageExchangeTimeMinutes,
                            int redeemTimeMinutes,
                            int mwcConfirmationNumber,
                            int secondaryConfirmationNumber,
                            QString communicationMethod,
                            QString communicationAddress ) :
            Mwc713Task("TaskCreateNewSwapTrade",
                       "swap_start --message_exchange_time " + QString::number(messageExchangeTimeMinutes) +
                           " --min_conf " + QString::number(min_confirmations) +
                           " --mwc_amount " + QString::number(mwc) +
                           " --mwc_confirmations " + QString::number(mwcConfirmationNumber) +
                           " --secondary_confirmations " + QString::number(secondaryConfirmationNumber) +
                           " --redeem_time " + QString::number(redeemTimeMinutes) +
                           " --secondary_address " + redeemAddress +
                           " --secondary_amount " + QString::number(btc) +
                           " --secondary_currency " + secondary +
                           " --method " + communicationMethod +
                           " --dest " + communicationAddress +
                           " --who_lock_first " + (sellerLockFirst ? "seller" : "buyer"),
                           wallet713, "") {}

    virtual ~TaskCreateNewSwapTrade() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};

// Cancel swap trade.
class TaskCancelSwapTrade : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*4;

    TaskCancelSwapTrade( MWC713 *wallet713, QString _swapId ) :
            Mwc713Task("TaskCancelSwapTrade",
                       "swap --adjust cancel -i " + _swapId,
                       wallet713, ""),
            swapId(_swapId) {}

    virtual ~TaskCancelSwapTrade() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString swapId;
};

// Cancel swap trade and swap trade details.
class TaskTradeDetails : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*10;

    TaskTradeDetails( MWC713 *wallet713, QString _swapId ) :
            Mwc713Task("TaskTradeDetails",
                       "swap --check  --json_format -i " + _swapId,
                       wallet713, ""),
            swapId(_swapId) {}

    virtual ~TaskTradeDetails() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString swapId;
};

// Adjust swap trade record.
class TaskAdjustTrade : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*10;

    TaskAdjustTrade( MWC713 *wallet713, const QString & _swapId, const QString &adjustCmd, const QString & param1, const QString & param2 ) :
            Mwc713Task("TaskTradeDetails",
                       generateCommandLine(_swapId, adjustCmd, param1, param2),
                       wallet713, ""),
            swapId(_swapId), adjustCommand(adjustCmd) {}

    virtual ~TaskAdjustTrade() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString generateCommandLine(const QString & swapId, const QString & adjustCmd, const QString & param1, const QString & param2) const;
private:
    QString swapId;
    QString adjustCommand;
};

// Perform auto swap single step
class TaskPerformAutoSwapStep : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*20;

    TaskPerformAutoSwapStep( MWC713 *wallet713, const QString & _swapId ) :
                Mwc713Task("TaskTradeDetails",
                    "swap --autoswap --json_format -i " + _swapId,
                    wallet713, ""),
    swapId(_swapId) {}

    virtual ~TaskPerformAutoSwapStep() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString swapId;
};

class TaskBackupSwapTradeData : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*2;

    TaskBackupSwapTradeData( MWC713 *wallet713, const QString & _swapId, const QString & backupFileName ) :
            Mwc713Task("TaskBackupSwapTradeData",
                    "swap --trade_export \"" + backupFileName + "\"  -i " + _swapId,
                    wallet713, ""),
    swapId(_swapId) {}

    virtual ~TaskBackupSwapTradeData() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString swapId;
};

class TaskRestoreSwapTradeData : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*3;

    TaskRestoreSwapTradeData( MWC713 *wallet713, const QString & _fileName ) :
            Mwc713Task("TaskBackupSwapTradeData",
                       "swap --trade_import \"" + _fileName + "\"",
                       wallet713, ""),
            fileName(_fileName) {}

    virtual ~TaskRestoreSwapTradeData() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString fileName;
};



}

#endif //MWC_QT_WALLET_TASKSWAP_H

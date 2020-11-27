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
    const static int64_t TIMEOUT = 1000*120; // 120 seconds can be possible because of the status update and connection errors

    TaskGetSwapTrades( MWC713 *wallet713, QString _cookie ) :
                Mwc713Task("TaskSwapTrades",
                "swap --list --check --json_format",
                wallet713, ""),
                cookie(_cookie) {}

    virtual ~TaskGetSwapTrades() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString cookie;
};

// Delete single trade.
class TaskDeleteSwapTrade : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*120;

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
    const static int64_t TIMEOUT = 1000*120; // In case of network issues, it will take time for retry

    TaskCreateNewSwapTrade( MWC713 *wallet713,
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
                            bool _dryRun,
                            QString _tag,
                            QVector<QString> _params ) :
            Mwc713Task("TaskCreateNewSwapTrade",
                       generateCommandLine(min_confirmations,mwcAmount, secAmount, secondary,
                                    redeemAddress, secTxFee, sellerLockFirst, messageExchangeTimeMinutes,
                                    redeemTimeMinutes, mwcConfirmationNumber, secondaryConfirmationNumber,
                                    communicationMethod, communicationAddress, electrum_uri1,
                                    electrum_uri2, _dryRun),
                       wallet713, ""), tag(_tag), dryRun(_dryRun), params(_params) {}

    virtual ~TaskCreateNewSwapTrade() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString generateCommandLine(int min_confirmations,
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
                                bool dryRun) const;

    QString tag;
    bool dryRun;
    QVector<QString> params;
};

// Cancel swap trade.
class TaskCancelSwapTrade : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*120;

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
    const static int64_t TIMEOUT = 1000*120;

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
    const static int64_t TIMEOUT = 1000*120;

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
    const static int64_t TIMEOUT = 1000*120;

    TaskPerformAutoSwapStep( MWC713 *wallet713, const QString & _swapId ) :
                Mwc713Task("TaskPerformAutoSwapStep_" + _swapId,
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
    const static int64_t TIMEOUT = 1000*120;

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
    const static int64_t TIMEOUT = 1000*120;

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


class TaskAdjustTradeState : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*120;

    TaskAdjustTradeState( MWC713 *wallet713, const QString & swapId, const QString & newState ) :
            Mwc713Task("TaskAdjustTradeState",
                       "swap --adjust " + newState + " -i " + swapId,
                       wallet713, "") {}

    virtual ~TaskAdjustTradeState() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};


}

#endif //MWC_QT_WALLET_TASKSWAP_H

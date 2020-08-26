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

#ifndef MWC_QT_WALLET_SWAP_S_H
#define MWC_QT_WALLET_SWAP_S_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QMap>
#include <QSet>

namespace state {

struct AutoswapTask {
    QString swapId;
    int64_t lastUpdatedTime = 0;

    void setData(QString _swapId, int64_t _lastUpdatedTime) { swapId = _swapId; lastUpdatedTime = _lastUpdatedTime; }
};

class Swap : public QObject, public State {
Q_OBJECT
public:
    Swap(StateContext * context);
    virtual ~Swap() override;

    // Show first page with trade List
    void pageTradeList();
    // New Trade Page
    void startNewTrade();
    // Edit/View Trade Page
    void viewTrade(QString swapId);
    // Show trade details page
    void showTradeDetails(QString swapId);

    // Return true if trade is running
    bool isTradeRunning(QString swapId);
    // Run the trade
    void runTrade(QString swapId);
    // Stop the trade
    void stopTrade(QString swapId);
private:
signals:

    void onSwapTradeStatusUpdated(QString swapId, QString currentAction, QString currentState,
                               QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                               QVector<wallet::SwapJournalMessage> tradeJournal);

protected:
    virtual NextStateRespond execute() override;

private:
    virtual void timerEvent(QTimerEvent *event) override;

private
slots:

    void onPerformAutoSwapStep(QString swapId, bool swapIsDone, QString currentAction, QString currentState,
                               QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                               QVector<wallet::SwapJournalMessage> tradeJournal,
                               QString error );

    void onNewSwapTrade(QString currency, QString swapId);

private:
    // Key: swapId,  Value: running Task
    QMap<QString, AutoswapTask> runningSwaps;
    QString  runningTask;

    QSet<QString> shownMessages;
};

}

#endif //MWC_QT_WALLET_SWAP_H

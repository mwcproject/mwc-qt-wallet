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

#include "s_swap.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../util/Log.h"

namespace state {

Swap::Swap(StateContext * context) :
        State(context, STATE::SWAP)
{
    QObject::connect( context->wallet, &wallet::Wallet::onPerformAutoSwapStep, this, &Swap::onPerformAutoSwapStep, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onNewSwapTrade, this, &Swap::onNewSwapTrade, Qt::QueuedConnection );

    startTimer(1000); // 1 second timer is fine. Timer is for try.
}

Swap::~Swap() {

}

NextStateRespond Swap::execute() {
    if (context->appContext->getActiveWndState() != STATE::SWAP)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    core::getWndManager()->pageSwapList();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// Show first page with trade List
void Swap::pageTradeList() {
    core::getWndManager()->pageSwapList();
}

// New Trade Page
void Swap::startNewTrade() {
    core::getWndManager()->pageSwapNew();
}

// Edit/View Trade Page
void Swap::viewTrade(QString swapId) {
    core::getWndManager()->pageSwapEdit(swapId);
}

// Show trade details page
void Swap::showTradeDetails(QString swapId) {
    core::getWndManager()->pageSwapTradeDetails(swapId);
}

// Return true if trade is running
bool Swap::isTradeRunning(QString swapId) {
    return runningSwaps.contains(swapId);
}

// Run the trade
void Swap::runTrade(QString swapId) {
    AutoswapTask task;
    task.setData(swapId, 0);
    runningSwaps.insert(swapId, task);
}

// Stop the trade
void Swap::stopTrade(QString swapId) {
    runningSwaps.remove(swapId);
}

void Swap::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event)

    if (runningSwaps.isEmpty() || !runningTask.isEmpty())
        return;

    int64_t curTime = QDateTime::currentSecsSinceEpoch();

    AutoswapTask nextTask;
    nextTask.lastUpdatedTime = curTime;

    for ( auto i = runningSwaps.constBegin(); i != runningSwaps.constEnd(); ++i ) {
        if ( i.value().lastUpdatedTime < nextTask.lastUpdatedTime ) {
            nextTask = i.value();
        }
    }

    // Let's run every minute, it should be enough
    if (curTime - nextTask.lastUpdatedTime > 60) {
        // starting the task
        runningTask = nextTask.swapId;
        runningSwaps[nextTask.swapId].lastUpdatedTime = curTime;
        context->wallet->performAutoSwapStep(nextTask.swapId);
    }
}

void Swap::onPerformAutoSwapStep(QString swapId, bool swapIsDone, QString currentAction, QString currentState,
                           QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                           QVector<wallet::SwapJournalMessage> tradeJournal,
                           QString error ) {

    // Checking if need to show deposit message for the buyer
    if (currentAction.contains("Please deposit exactly")) {
        if (!shownMessages.contains(currentAction)) {
            shownMessages.insert(currentAction);
            core::getWndManager()->messageTextDlg("Trade " + swapId, currentAction);
        }
    }

    if (swapId != runningTask)
        return;

    runningTask = "";

    // Running task is executed, let's update it
    if (!error.isEmpty()) {
        core::getWndManager()->messageTextDlg("Swap Processing Error", "Autoswap step is failed for swap " + swapId + "\n\n" + error );
        return;
    }

    if (swapIsDone) {
        runningSwaps.remove(swapId);
    }

    logger::logEmit( "SWAP", "onSwapTradeStatusUpdated", swapId + ", " + currentAction + ", " + currentState );
    emit onSwapTradeStatusUpdated( swapId, currentAction, currentState, executionPlan, tradeJournal);
}

void Swap::onNewSwapTrade(QString currency, QString swapId) {
    core::getWndManager()->messageTextDlg("New Swap Offer",
             "You get a new Swap Offer to Buy " + currency + " coins for your MWC.\nTrade SwapId: " + swapId +
             "\n\nPlease reviews this offer befo run is and accept. Please check if all details like amounts, lock order, confirmation number are meet your expectations.\n\n"
             "Please remember that number of confirmations should match the amount.\n"
             "During the trade process the funds are locked. If other party abandon the trade, your funds will be locked for some time.\n\n"
             "During the whole wap trade process please keep your wallet online and this trade 'Running'");
}



}

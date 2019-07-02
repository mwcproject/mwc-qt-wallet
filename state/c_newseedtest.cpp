#include "state/c_newseedtest.h"
#include "../wallet/wallet.h"
#include "../windows/c_newseedtest_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QVector>
#include "../core/testseedtask.h"
#include "../control/messagebox.h"
#include "../util/Log.h"

namespace state {

NewSeedTest::NewSeedTest(const StateContext & context) :
    State( context, STATE::TEST_NEW_SEED )
{
    QObject::connect( context.wallet, &wallet::Wallet::onWalletBalanceUpdated,
                                 this, &NewSeedTest::onWalletBalanceUpdated, Qt::QueuedConnection );
}

NewSeedTest::~NewSeedTest() {
}


NextStateRespond NewSeedTest::execute() {

    QVector< core::TestSeedTask > task =
            context.appContext->getCookie< QVector<core::TestSeedTask> >("seedTasks");

    if (task.size()==0) {
        QVector<QString> seed = context.appContext->getCookie< QVector<QString> >("seed2verify");
        // It was the last challenge. Need to acknoledge wallet that user is done with a passphrase
        if (seed.size()>0) {
            // clean up the state
            context.appContext->pullCookie< QVector<QString> >("seed2verify");
            context.wallet->confirmNewSeed();

            control::MessageBox::message(nullptr, "Congratulations!", "Thank you for confirming all words from your passphrase. Your wallet was successfully created");

            // Updating the wallet balance
            context.wallet->updateWalletBalance();

            return NextStateRespond(NextStateRespond::RESULT::WAIT_FOR_ACTION);
        }

        return NextStateRespond(NextStateRespond::RESULT::DONE);
    }

    // Show verify dialog
    currentTask = task.last();
    task.pop_back();
    context.appContext->pushCookie< QVector< core::TestSeedTask > >("seedTasks", task);

    wnd = new wnd::NewSeedTest( context.wndManager->getInWndParent(), this, currentTask.getWordIndex() );
    context.wndManager->switchToWindow( wnd );

    return NextStateRespond(NextStateRespond::RESULT::WAIT_FOR_ACTION);
}

// Account info is updated
void NewSeedTest::onWalletBalanceUpdated() {

    if (!wnd) // wnd as active state
        return;

    context.stateMachine->executeFrom(STATE::TEST_NEW_SEED);
}


void NewSeedTest::submit(QString word) {
    if (currentTask.applyInputResults(word)) {
        // ok case
        context.stateMachine->executeFrom(STATE::TEST_NEW_SEED);
        return;
    }

    control::MessageBox::message(nullptr, "Wrong word",
              "The word number " + QString::number(currentTask.getWordIndex()) + " was typed incorrectly. " +
              "Please review your passphrase.");

    // regenerate if totally failed
    if (currentTask.isTestCompletelyFailed()) {
        // generate a new seed for a new wallet
        QVector<QString> seed = context.appContext->getCookie< QVector<QString> >("seed2verify");
        context.appContext->pushCookie< QVector<core::TestSeedTask> >("seedTasks", core::generateSeedTasks( seed ) );
    }
    else {
        // add to the Q
        QVector< core::TestSeedTask > task =
                context.appContext->getCookie< QVector<core::TestSeedTask> >("seedTasks");
        task.push_front(currentTask);
        context.appContext->pushCookie< QVector< core::TestSeedTask > >("seedTasks", task);
    }

    context.stateMachine->executeFrom(STATE::SHOW_NEW_SEED);
}


};

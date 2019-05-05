#include "state/newseedtest.h"
#include "../wallet/wallet.h"
#include "../windows/newseedtest_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QMessageBox>
#include <QVector>


namespace state {

NewSeedTest::NewSeedTest(const StateContext & context) :
    State( context, STATE::TEST_NEW_SEED )
{
}

NewSeedTest::~NewSeedTest() {
}


NextStateRespond NewSeedTest::execute() {

    QVector< QPair<int,QString> > task =
            context.appContext->getCookie< QVector< QPair<int,QString> > >("seedTasks");

    if (task.size()==0) {
        QVector<QString> seed = context.appContext->getCookie< QVector<QString> >("seed2verify");
        // It was the last challenge. Need to acknoledge wallet that user is done with a passphrase
        if (seed.size()>0) {
            // clean up the state
            context.appContext->pullCookie< QVector<QString> >("seed2verify");
            context.wallet->confirmNewSeed();

            QMessageBox::information(nullptr, "Seed", "Thank you for confirming all words from your seed. Your wallet was successfully created");
        }

        return NextStateRespond(NextStateRespond::RESULT::DONE);
    }

    // Show verify dialog
    currentTask = task.last();
    task.pop_back();
    context.appContext->pushCookie< QVector< QPair<int,QString> > >("seedTasks", task);

    context.wndManager->switchToWindow(
                new wnd::NewSeedTest( context.wndManager->getInWndParent(), this, currentTask.first ) );

    return NextStateRespond(NextStateRespond::RESULT::WAIT_FOR_ACTION);
}


void NewSeedTest::submit(QString word) {
    if (word==currentTask.second) {
        // ok case
        context.stateMachine->executeFrom(STATE::TEST_NEW_SEED);
        return;
    }

    QMessageBox::information(nullptr, "Wrong word",
              "The word number " + QString::number(currentTask.first) + " was typed incorrectly. " +
              "Please review your passphrase.");
    context.stateMachine->executeFrom(STATE::SHOW_NEW_SEED);
}


};

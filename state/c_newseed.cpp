#include "c_newseed.h"
#include "../wallet/wallet.h"
#include "../windows/c_newseed_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/testseedtask.h"
#include "../util/Log.h"

namespace state {


NewSeed::NewSeed( StateContext * context) :
    State(context, STATE::GENERATE_NEW_SEED)
{
    QObject::connect( context->wallet, &wallet::Wallet::onNewSeed, this, &NewSeed::onNewSeed, Qt::QueuedConnection );
}

NewSeed::~NewSeed(){
}


NextStateRespond NewSeed::execute() {
    QString newSeed = context->appContext->pullCookie<QString>("newSeed");
    if (newSeed.length()==0) {
        return NextStateRespond(NextStateRespond::RESULT::DONE);
    }

    // Need a password for generation. Using destructive get to keep memory clean
    QString password = context->appContext->pullCookie<QString>(COOKIE_PASSWORD);
    if (password.isEmpty()) {
        Q_ASSERT(false); // Not expected
        context->wallet->reportFatalError("Internal Error at a NewSeed state");

        return NextStateRespond(NextStateRespond::RESULT::DONE);
    }


    // generate a new seed for a new wallet
    context->wallet->generateSeedForNewAccount(password);

    // No windows are created!!!
    // Expected that it is a short operation
    return NextStateRespond(NextStateRespond::RESULT::WAIT_FOR_ACTION);
}


void NewSeed::onNewSeed(QVector<QString> seed) {
    // Get the seed, continue processing
    context->appContext->pushCookie< QVector<QString> >("seed2verify", seed);
    context->appContext->pushCookie< QVector<core::TestSeedTask> >("seedTasks", core::generateSeedTasks( seed ) );

    context->stateMachine->executeFrom(STATE::GENERATE_NEW_SEED);
}


}

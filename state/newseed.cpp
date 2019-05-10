#include "newseed.h"
#include "../wallet/wallet.h"
#include "../windows/newseed_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QMessageBox>
#include <QRandomGenerator>
#include "../core/testseedtask.h"

namespace state {

NewSeed::NewSeed(const StateContext & context) :
    State(context, STATE::GENERATE_NEW_SEED)
{
}

NewSeed::~NewSeed(){
}


NextStateRespond NewSeed::execute() {
    QString newSeed = context.appContext->pullCookie<QString>("newSeed");
    if (newSeed.length()==0) {
        return NextStateRespond(NextStateRespond::RESULT::DONE);
    }

    // generate a new seed for a new wallet
    QVector<QString> seed = context.wallet->init();
    context.appContext->pushCookie< QVector<QString> >("seed2verify", seed);

    context.appContext->pushCookie< QVector<core::TestSeedTask> >("seedTasks", core::generateSeedTasks( seed ) );

    return NextStateRespond(NextStateRespond::RESULT::DONE);
}

void NewSeed::submit() {
}


}

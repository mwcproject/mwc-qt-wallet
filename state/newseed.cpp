#include "newseed.h"
#include "../wallet/wallet.h"
#include "../windows/newseed_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QMessageBox>
#include <QRandomGenerator>

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

    // Generate tasks. Need to review the words one by one
    QVector< QPair<int,QString> > words;
    for ( int i=0;i<seed.size();i++ )
        words.push_back( QPair<int,QString>(i+1, seed[i]) );

    QVector< QPair<int,QString> > confirmTasks;
    QRandomGenerator * rand = QRandomGenerator::global();

    while( words.size()>0 ) {
        int idx = rand->bounded( words.size() );
        confirmTasks.push_back( words[idx] );
        words.remove(idx);
    }
    context.appContext->pushCookie< QVector< QPair<int,QString> > >("seedTasks", confirmTasks);

    return NextStateRespond(NextStateRespond::RESULT::DONE);
}

void NewSeed::submit() {
}


}

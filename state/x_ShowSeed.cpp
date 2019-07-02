#include "x_ShowSeed.h"
#include "../windows/c_newseed_w.h"
#include "../core/windowmanager.h"

namespace state {

ShowSeed::ShowSeed(const StateContext & context) : State(context,  STATE::SHOW_SEED ) {
    QObject::connect(context.wallet, &wallet::Wallet::onGetSeed,
                                   this, &ShowSeed::recoverPassphrase, Qt::QueuedConnection);

}

ShowSeed::~ShowSeed() {}

NextStateRespond ShowSeed::execute() {
    if ( context.appContext->getActiveWndState() != STATE::SHOW_SEED )
        return NextStateRespond(NextStateRespond::RESULT::DONE);


    wnd = new wnd::NewSeed( context.wndManager->getInWndParent(), this, getStateMachine(),  QVector<QString>(), false );
    context.wndManager->switchToWindow( wnd );

    context.wallet->getSeed();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void ShowSeed::recoverPassphrase( QVector<QString> seed ) {
    if (wnd) {
        wnd->showSeedData(seed);
    }
}


}

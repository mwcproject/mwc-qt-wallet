#include "a_StartWallet.h"
#include "../core/appcontext.h"
#include "../core/windowmanager.h"
#include "../state/statemachine.h"
#include "../windows/a_waitingwnd.h"
#include "../util/Log.h"

namespace state {

// Init the wallet. Then check how it is started. If it needs to have password or something
StartWallet::StartWallet(const StateContext & context) :
        State(context, STATE::START_WALLET)
{}

StartWallet::~StartWallet() {
}

NextStateRespond StartWallet::execute() {
    wallet::InitWalletStatus status = context.wallet->getWalletStatus();

    if (status == wallet::InitWalletStatus::READY)
        return NextStateRespond(NextStateRespond::RESULT::DONE); // We are good, nothing need to be done

    if (status == wallet::InitWalletStatus::NONE) {
        // starting the wallet. Then will check what we need

        context.wndManager->switchToWindow(
                new wnd::WaitingWnd( context.wndManager->getInWndParent(),
                        "Starting mwc713", "Please wait for mwc713 process starting..." ) );

        log::logConnect("StartWallet", "onInitWalletStatus" );
        slotConn = QObject::connect( context.wallet, &wallet::Wallet::onInitWalletStatus, this, &StartWallet::onInitWalletStatus, Qt::QueuedConnection );

        context.wallet->start();
        return NextStateRespond(NextStateRespond::RESULT::WAIT_FOR_ACTION);
    }

    // InputPassword will handle  NEED_PASSWORD & WRONG_PASSWORD
    // NextStateRespond will handle NEED_INIT
    return NextStateRespond(NextStateRespond::RESULT::DONE);
}


void StartWallet::onInitWalletStatus(wallet::InitWalletStatus status) {
    log::logRecieve("StartWallet", "onInitWalletStatus", toString(status) );
    // Listen for the changes...

    switch (status) {
        case wallet::InitWalletStatus::READY:
        case wallet::InitWalletStatus::NEED_INIT:
        case wallet::InitWalletStatus::NEED_PASSWORD: {
            QObject::disconnect(slotConn);
            log::logDisconnect("StartWallet", "onInitWalletStatus" );
            context.stateMachine->executeFrom(STATE::STATE_INIT);
            return;
        }
        default: {
            Q_ASSERT(false);
            context.wallet->reportFatalError("Unable to init the wallet. Get unexpected wallet status: " + QString::number(status));
            return;
        }
    }
}



}

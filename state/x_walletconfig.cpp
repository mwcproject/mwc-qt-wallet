#include "x_walletconfig.h"
#include "../wallet/wallet.h"
#include "../windows/x_walletconfig_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

WalletConfig::WalletConfig(const StateContext & context) :
    State (context, STATE::WALLET_CONFIG) {
}

WalletConfig::~WalletConfig() {}

NextStateRespond WalletConfig::execute() {
    if (context.appContext->getActiveWndState() != STATE::WALLET_CONFIG)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindowEx(
                new wnd::WalletConfig( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


wallet::WalletConfig WalletConfig::getWalletConfig() const {
    return context.wallet->getWalletConfig();
}

bool WalletConfig::setWalletConfig(const wallet::WalletConfig & config) {
    if (context.wallet->setWalletConfig(config)) {
        // restarting the wallet...
        context.stateMachine->executeFrom( STATE::NONE );
        return true;
    }
    return false;
}



core::SendCoinsParams  WalletConfig::getSendCoinsParams() const {
    return context.appContext->getSendCoinsParams();
}

// account refresh will be requested...
void WalletConfig::setSendCoinsParams(const core::SendCoinsParams & params) {
    context.appContext->setSendCoinsParams(params);
    context.wallet->updateWalletBalance(); // Number of outputs might change, requesting update in background
}



}


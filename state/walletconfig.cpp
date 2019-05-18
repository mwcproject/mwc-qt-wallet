#include "walletconfig.h"
#include "../wallet/wallet.h"
#include "../windows/walletconfig_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QMessageBox>

namespace state {

WalletConfig::WalletConfig(const StateContext & context) :
    State (context, STATE::WALLET_CONFIG) {
}

WalletConfig::~WalletConfig() {}

NextStateRespond WalletConfig::execute() {
    if (context.appContext->getActiveWndState() != STATE::WALLET_CONFIG)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::WalletConfig( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


wallet::WalletConfig WalletConfig::getWalletConfig() {
    return context.wallet->getWalletConfig();
}

QPair<bool, QString> WalletConfig::setWalletConfig(const wallet::WalletConfig & config) {
    return context.wallet->setWalletConfig(config);
}

}


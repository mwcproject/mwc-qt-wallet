// Copyright 2019 The MWC Developers
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

#include "state/a_inputpassword.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/WndManager.h"
#include <QCoreApplication>
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/a_inputpassword_b.h"
#include <QDir>
#include "../util/crypto.h"
#include "util/ioutils.h"

namespace state {

InputPassword::InputPassword( StateContext * _context) :
    State(_context, STATE::INPUT_PASSWORD)
{
    // Result of the login
    QObject::connect( context->wallet, &wallet::Wallet::onScanDone, this, &InputPassword::onScanDone, Qt::QueuedConnection );
}

InputPassword::~InputPassword() {
}

NextStateRespond InputPassword::execute() {
    bool running = context->wallet->isInit();
    QString lockStr = context->appContext->pullCookie<QString>("LockWallet");
    inLockMode = false;

    // Always try to start the wallet. State before is responsible for the first init
    if ( !running ) {
        // As an online node we can go forward because no password is expected
        if (config::isOnlineNode()) {
            core::getWndManager()->pageNodeInfo();
            // Starting the wallet normally for the node
            return NextStateRespond( NextStateRespond::RESULT::DONE );
        }

        core::getWndManager()->pageInputPassword(mwc::PAGE_A_ACCOUNT_LOGIN, false);

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    if (!lockStr.isEmpty()) {
        inLockMode = true;
        mwc::setWalletLocked(inLockMode);
        // wallet locking mode
        core::getWndManager()->pageInputPassword(mwc::PAGE_A_ACCOUNT_UNLOCK, true);
        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

bool InputPassword::submitPassword(const QString & password, const QString & selectedPath) {
    logger::logInfo(logger::STATE, "Call InputPassword::submitPassword with <password> selectedPath=" + selectedPath);
    bool running = context->wallet->isInit();

    QPair<bool,QString> baseFullPath = ioutils::getAppDataPath( selectedPath );
    if (!baseFullPath.first)
        return false;

    // Check if we can skip logout/login step.
    // If wallet wasn't changed, there is no reason logout and restart all listeners.
    if (inLockMode && running && context->wallet->getWalletConfig().getDataPath() == baseFullPath.second &&
                context->appContext->getActiveWndState() != STATE::SHOW_SEED) {
        // We can just verify the password and we should be good

        if (!context->wallet->checkPassword(password)) {
            core::getWndManager()->messageTextDlg("Password", "Password supplied was incorrect. Please input correct password.");
            return false;
        }

        // We are good here, let's unlock the wallet
        inLockMode = false;
        mwc::setWalletLocked(inLockMode);

        context->stateMachine->executeFrom(STATE::NONE);
        return true;
    }

    QVector<QString> walletInfo = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(selectedPath,  context->appContext);
    std::shared_ptr<node::NodeClient> nodeCLient;
    if (walletInfo.size()>=1) {
        QString network = walletInfo[0];
        if (context->wallet->getWalletConfig().getNetwork() == network)
            nodeCLient = context->wallet->getNodeClient();
    }

    // Check if we need to logout first. It is very valid case if we in lock mode
    context->wallet->logout();

    if (inLockMode) {
        inLockMode = false;
        mwc::setWalletLocked(inLockMode);
    }

    if (!context->initWalletNode(selectedPath, nodeCLient)) {
        logger::logError(logger::QT_WALLET, "Unable to init wallet for data location " + selectedPath);
        return false;
    }

    QString err = context->wallet->loginWithPassword( password );
    if (!err.isEmpty()) {
        logger::logDebug(logger::QT_WALLET, "Log attempt was failed. Error: " + err);
        core::getWndManager()->messageTextDlg("Login error", err);
        return false;
    }

    // Going forward by initializing the wallet
    if ( context->wallet->isInit() ) {
        if (! config::isOnlineNode()) {
            // Updating the wallet balance
            updateRespId = context->wallet->update_wallet_state();
        }

        // Starting listeners after balance to speed up the init process
        if ( config::isOnlineWallet() ) {
            // Start listening, no feedback interested
            context->wallet->listeningStart(context->appContext->isFeatureMWCMQS(),
                                            context->appContext->isFeatureTor());
        }
    }

    if ( context->appContext->getActiveWndState() == STATE::SHOW_SEED ) {
        context->appContext->pushCookie<QString>("password", password);
    }
    return true;
}

void InputPassword::onScanDone( QString responseId, bool fullScan, int height, QString errorMessage ) {
    if (updateRespId!=responseId)
        return;

    Q_UNUSED(fullScan);
    Q_UNUSED(height);
    Q_UNUSED(errorMessage);

    // Using wnd as a flag that we are active.
    if ( !inLockMode && state::getStateMachine()->getCurrentStateId() == STATE::INPUT_PASSWORD) {
        context->stateMachine->executeFrom(STATE::INPUT_PASSWORD);
    }
}

}

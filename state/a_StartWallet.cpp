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

#include "a_StartWallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../util/Process.h"
#include "../core/WndManager.h"
#include <QDir>

namespace state {

// Init the wallet. Then check how it is started. If it needs to have password or something
StartWallet::StartWallet(StateContext * context) :
        State(context, STATE::START_WALLET)
{
}

StartWallet::~StartWallet() {
}

NextStateRespond StartWallet::execute() {
    if ( !context->wallet->isRunning() || !context->appContext->getCookie<QString>("checkWalletInitialized").isEmpty() ) {

        if ( context->appContext->getCookie<QString>("checkWalletInitialized").isEmpty() ) {
            // Check what are the wallet instances we have.
            QPair<QVector<QString>, int> instances = context->appContext->getWalletInstances(true);
            if (instances.first.isEmpty()) {
                core::getWndManager()->pageInitFirstTime();
                return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
            } else {
                // Just update the wallet with a status. Then continue
                context->appContext->pushCookie<QString>("checkWalletInitialized",
                                                         context->wallet->checkWalletInitialized(true) ? "OK" : "FAILED");
            }
        }
    }

    return NextStateRespond(NextStateRespond::RESULT::DONE);
}

void StartWallet::createNewWalletInstance(QString path, bool restoreWallet) {
    // Check if this path already exist
    if (path.isEmpty()) {
        // Generating a new path for the wallet
        QPair<bool,QString> res = ioutils::getAppDataPath("");

        if (!res.first) {
            core::getWndManager()->messageTextDlg("Error", res.second);
            return;
        }

        QString basePath = res.second;
        QString emptyWalletDir;
        for ( int i=1; i<100; i++ ) {
            QString walletDataPath = "gui_wallet713_data" + QString::number(i);
            if (!wallet::WalletConfig::doesSeedExist(walletDataPath)) {
                emptyWalletDir = basePath + QDir::separator() + walletDataPath;
                break; // can use this directory as a new wallet
            }
        }

        if (emptyWalletDir.isEmpty()) {
            core::getWndManager()->messageTextDlg("Error", "There are too many wallets are created. We can't have another one");
            return;
        }

        QDir dir(emptyWalletDir);
        if (!dir.removeRecursively()) {
            core::getWndManager()->messageTextDlg("Error", "Unable to cleanup the directory for the new wallet " + emptyWalletDir);
            return;
        }

        QDir baseDir(basePath);
        emptyWalletDir = baseDir.relativeFilePath(emptyWalletDir);

        context->appContext->addNewInstance(emptyWalletDir);
    }
    else {
        QVector<QString> network_arch_name = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(path);
        Q_ASSERT(network_arch_name.size() ==3);

        QString arh = network_arch_name[1];
        if (arh != util::getBuildArch()) {
            core::getWndManager()->messageTextDlg("Error", "Wallet data at directory " + path +
                    " was belong to different architecture. Expecting " + util::getBuildArch() + " but get " + arh);
            return ;
        }

        context->appContext->addNewInstance(path);
    }


    context->appContext->pushCookie<bool>("restoreWalletFromSeed",restoreWallet);
    context->appContext->pushCookie<QString>("checkWalletInitialized",
                                             context->wallet->checkWalletInitialized(false) ? "OK" : "FAILED");
    context->stateMachine->executeFrom(STATE::START_WALLET);
}

}

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

#include "state.h"

#include <QCoreApplication>
#include <QFileInfo>

#include "statemachine.h"
#include "wallet/wallet_objs.h"
#include "wallet/wallet.h"
#include "node/MwcNode.h"
#include "node/node_client.h"
#include "util/ioutils.h"
#include "util/Log.h"
#include <QDir>

namespace state {

static StateContext * globalContext = nullptr;

void setStateContext(StateContext * context) {
    Q_ASSERT(globalContext == nullptr); // set it once
    globalContext = context;

    Q_ASSERT(globalContext);
}

StateContext * getStateContext() {
    Q_ASSERT(globalContext);
    return globalContext;
}

// Shortcut to state if state_machine. Callir suppose to know the class name
State * getState(STATE state) {
    State * res = getStateContext()->stateMachine->getState(state);
    Q_ASSERT(res);
    return res;
}

    StateContext::StateContext(core::AppContext * _appContext,
                     wallet::Wallet * _wallet, QFuture<QString> * _torStarter) :
            appContext(_appContext), wallet(_wallet), stateMachine(nullptr), torStarter(_torStarter) {
    Q_ASSERT(appContext);
    Q_ASSERT(wallet);
    Q_ASSERT(torStarter);
}


// Init node & wallet for a well defined path
bool StateContext::isWalletDataValid(const QString & basePath, bool hasSeed) {
    QPair<bool,QString> baseFullPath = ioutils::getAppDataPath( basePath );
    if (!baseFullPath.first)
        return false;

    if (hasSeed && QFileInfo(baseFullPath.second + QDir::separator() + "wallet.seed").size() < 100 )
        return false;

    QVector<QString> walletInfo = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(basePath, appContext);
    // walletInfo data: [Network, Arch, InstanceName]
    return walletInfo.size()>=1;
}

// Will read the network from the base path
bool StateContext::initWalletNode(const QString & basePath, std::shared_ptr<node::NodeClient> nodeClient) {
    QVector<QString> walletInfo = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(basePath, appContext);
    if (walletInfo.size()>=1) {
        return initWalletNode(basePath, walletInfo[0], nodeClient);
    }
    return false;
}

// Dont expect any wallet info in the base Path (online network case)
bool StateContext::initWalletNode(const QString & basePath, const QString & network, std::shared_ptr<node::NodeClient> nodeClient) {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    QPair<bool,QString> baseFullPath = ioutils::getAppDataPath( basePath );
    if (!baseFullPath.first)
        return false;

    if (wallet->isInit() && wallet->getWalletConfig().getNetwork()==network) {
        nodeClient = wallet->getNodeClient();
    }

    wallet->logout();
    Q_ASSERT(wallet->getContextId()<0);

    if (nodeClient != nullptr) {
       Q_ASSERT( nodeClient->getCurrentNetwork() == network );
    }

    if (nodeClient==nullptr) {
        QPair<bool,QString> nodeDataPath = ioutils::getAppDataPath("mwc-node/" + network, true);
        Q_ASSERT(nodeDataPath.first);
        if (!nodeDataPath.first) {
            Q_ASSERT(false);
            // embedding node is not critical, not exiting the wallet
            logger::logError(logger::QT_WALLET, "Unable initialize directory for embedded node");
        }

        nodeClient = std::shared_ptr<node::NodeClient>(new node::NodeClient( nodeDataPath.second, network, appContext, torStarter));
    }
    Q_ASSERT(nodeClient!=nullptr);
    Q_ASSERT(wallet->getContextId()<0);
    wallet->init(network, baseFullPath.second, nodeClient);

    return true;
}


State::State(StateContext * _context, STATE _stateId) :
    context(_context), stateId(_stateId)
{}

State::~State() {}

}

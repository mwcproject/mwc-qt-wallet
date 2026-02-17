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
                     wallet::Wallet * _wallet,
                     node::MwcNode * _mwcNode) :
            appContext(_appContext), wallet(_wallet), mwcNode(_mwcNode), stateMachine(nullptr) {
    Q_ASSERT(appContext);
    Q_ASSERT(wallet);
    Q_ASSERT(mwcNode);
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
bool StateContext::initWalletNode(const QString & basePath) {
    QVector<QString> walletInfo = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(basePath, appContext);
    if (walletInfo.size()>=1) {
        return initWalletNode(basePath, walletInfo[0]);
    }
    return false;
}

// Dont expect any wallet info in the base Path (online network case)
bool StateContext::initWalletNode(const QString & basePath, const QString & network) {
    QPair<bool,QString> baseFullPath = ioutils::getAppDataPath( basePath );
    if (!baseFullPath.first)
        return false;

    wallet->logout();

    Q_ASSERT(mwcNode);
    if (mwcNode->isRunning() && mwcNode->getCurrentNetwork() != network) {
        Q_ASSERT(nodeClient != nullptr);
        nodeClient->deleteLater();
        nodeClient = nullptr;
        mwcNode->stop();
    }

    // Starting new embedded node
    if (!mwcNode->isRunning()) {
        QPair<bool,QString> nodeDataPath = ioutils::getAppDataPath("mwc-node/" + network, true);
        Q_ASSERT(nodeDataPath.first);
        if (!nodeDataPath.first) {
            // embedding node is not critical, not exiting the wallet
            logger::logError(logger::QT_WALLET, "Unable initialize directory for embedded node");
            mwcNode->reportNodeError("Unable initialize directory for embedded node");
        }
        else {
            mwcNode->start( nodeDataPath.second , network );
        }

        Q_ASSERT(nodeClient == nullptr);
        nodeClient = new node::NodeClient(network, mwcNode, wallet);
    }
    Q_ASSERT(nodeClient);

    wallet->init(network, baseFullPath.second, nodeClient);

    walletPasePath = basePath;

    return true;
}


State::State(StateContext * _context, STATE _stateId) :
    context(_context), stateId(_stateId)
{}

State::~State() {}

}

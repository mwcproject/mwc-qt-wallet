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

#include "u_nodeinfo.h"
#include "../windows/u_nodeinfo_w.h"
#include "../wallet/wallet.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QDebug>
#include "../util/Log.h"
#include "../core/global.h"

namespace state {

void NodeStatus::setData(bool _online,
             const QString & _errMsg,
             int _nodeHeight,
             int _peerHeight,
             int64_t _totalDifficulty,
             int _connections) {
    online = _online;
    errMsg = _errMsg;
    nodeHeight = _nodeHeight;
    peerHeight = _peerHeight;
    totalDifficulty = _totalDifficulty;
    connections = _connections;
}

////////////////////////////////////////////////////

NodeInfo::NodeInfo(StateContext * context) :
        State(context, STATE::NODE_INFO )
{
    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &NodeInfo::onNodeStatus, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onLoginResult,
                     this, &NodeInfo::onLoginResult, Qt::QueuedConnection);

    // Checking/update node status every 20 seconds...
    startTimer(20000); // Let's update node info every 20 seconds
}

NodeInfo::~NodeInfo() {

}


NextStateRespond NodeInfo::execute() {
    if ( context->appContext->getActiveWndState() != STATE::NODE_INFO )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        wnd = (wnd::NodeInfo*) context->wndManager->switchToWindowEx( mwc::PAGE_U_NODE_STATUS,
                new wnd::NodeInfo( context->wndManager->getInWndParent(), this));

        wnd->setNodeStatus( lastNodeStatus );
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// After login - let's check the node status
void NodeInfo::onLoginResult(bool ok) {
    if (ok) {
        requestNodeInfo();
        justLogin = true;
    }
}


void NodeInfo::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);

    // Don't request for inti or lock states.
    if ( context->stateMachine->getActionWindow() >= STATE::ACCOUNTS )
        requestNodeInfo();
}

void NodeInfo::requestWalletResync() {
    context->appContext->pushCookie("PrevState", (int)context->appContext->getActiveWndState() );
    context->stateMachine->setActionWindow( state::STATE::RESYNC );
}

void NodeInfo::requestNodeInfo() {
    context->wallet->getNodeStatus();
}

wallet::WalletConfig NodeInfo::getWalletConfig() const {
    return context->wallet->getWalletConfig();
}

void NodeInfo::updateWalletConfig( const wallet::WalletConfig & config ) {
    if ( context->wallet->setWalletConfig(config) ) {
        // config require to restart
        context->stateMachine->executeFrom( STATE::NONE );
    }
}

void NodeInfo::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {

    lastNodeStatus.setData(online, errMsg, nodeHeight, peerHeight, totalDifficulty, connections);

    if (justLogin) {
        justLogin = false;
        // Let's consider 5 blocks (5 minutes) unsync be critical issue
        if ( !online || nodeHeight < peerHeight - mwc::NODE_HEIGHT_DIFF_LIMIT || connections==0 ) {
            if (wnd == nullptr) {
                // Switching to this Node Info state. State switch will take care about the rest workflow
                context->stateMachine->setActionWindow( state::STATE::NODE_INFO );
                return;
            }
        }
    }

    if (wnd== nullptr)
        return;

    wnd->setNodeStatus( lastNodeStatus );
}


}

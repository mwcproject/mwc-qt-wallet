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
#include "../core/Notification.h"
#include "../node/MwcNode.h"

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

    QObject::connect(context->mwcNode, &node::MwcNode::onMwcOutputLine,
                     this, &NodeInfo::onMwcOutputLine, Qt::QueuedConnection);

    // Checking/update node status every 20 seconds...
    startTimer(3000); // Let's update node info every 60 seconds. By some reasons it is slow operation...
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
        updateMwcNodeForWnd();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// After login - let's check the node status
void NodeInfo::onLoginResult(bool ok) {
    if (ok) {
        currentNodeConnection = getNodeConnection().first;
        requestNodeInfo();
        justLogin = true;
    }
}


void NodeInfo::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);

    timerCounter++;

    // Don't request for init or lock states.
    if ( context->stateMachine->getCurrentStateId() >= STATE::ACCOUNTS ) {
        int div = 1;
        if ( currentNodeConnection.connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD ) {
            // timer is 3 seconds.
            div = 20; // want to update onece in a minute
        }
        else if (!lastNodeStatus.online) {
            div = 5; // oflline node, doesn't make sense to update too often
        }
        else if (lastNodeStatus.connections==0) {
            div = 5; // no peers, likely an issue,  doesn't make sense to update too often
        }
        else if (lastNodeStatus.nodeHeight < lastNodeStatus.peerHeight - 3) {
            // must be in sync mode...
            div = 1;
        }
        else {
            // normal running mode, local node
            div =3;
        }

        if (timerCounter%div == 0) {
            requestNodeInfo();
        }
    }
}

void NodeInfo::requestWalletResync() {
    context->appContext->pushCookie("PrevState", (int)context->appContext->getActiveWndState() );
    context->stateMachine->setActionWindow( state::STATE::RESYNC );
}

void NodeInfo::requestNodeInfo() {
    context->wallet->getNodeStatus();
}

QPair< wallet::MwcNodeConnection, wallet::WalletConfig > NodeInfo::getNodeConnection() const {
    wallet::WalletConfig wltConfig = context->wallet->getWalletConfig();
    return QPair< wallet::MwcNodeConnection, wallet::WalletConfig >(  context->appContext->getNodeConnection( wltConfig.getNetwork() )  , wltConfig );
}

void NodeInfo::updateNodeConnection( const wallet::MwcNodeConnection & nodeConnect, const wallet::WalletConfig & walletConfig ) {
    context->appContext->updateMwcNodeConnection( walletConfig.getNetwork(), nodeConnect );
    context->wallet->setWalletConfig( walletConfig, context->appContext, context->mwcNode );
    // config require to restart
    currentNodeConnection = nodeConnect;
    context->stateMachine->executeFrom( STATE::NONE );
}

void NodeInfo::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {

    if (!justLogin) {
        // check if node state was changed. In this case let's emit a message
        if (online != lastNodeStatus.online) {
            if (online) {
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                        "Wallet restore connection to mwc node");
            }
            else {
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::CRITICAL,
                                                           "Wallet lost connection to mwc node");
            }
        }
        else if ( (connections==0) ^ (lastNodeStatus.connections==0) ) {
                if (connections>0) {
                    notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                               "mwc node restored connection to mwc network");
                }
                else {
                    notify::appendNotificationMessage(notify::MESSAGE_LEVEL::CRITICAL,
                                                               "mwc node lost connection to mwc network");
                }
        }
        else if ( (nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < peerHeight) ^ (lastNodeStatus.nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < lastNodeStatus.peerHeight) ) {
            if (nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < peerHeight) {
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::CRITICAL,
                                                           "mwc node out of sync from mwc network");
            }
            else {
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                           "mwc node finish syncing and runs well now");
            }
        }
    }

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

void NodeInfo::updateMwcNodeForWnd() {
    if (wnd!= nullptr) {
        int lnNum = wnd->getLogLineNumber();
        QStringList lns;
        for ( int i = std::max(0, logLines.size() - lnNum); i<logLines.size(); i++ ) {
            lns.push_back(logLines.at(i));
        }

        wnd->updateEmbeddedMwcNodeLogs(lns.join("\n"));
    }
}


void NodeInfo::onMwcOutputLine(QString line) {
    logLines.push_back(line);

    while(logLines.size()>100)
        logLines.pop_front();

    updateMwcNodeForWnd();
}


}

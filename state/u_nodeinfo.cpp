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
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QDebug>
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Notification.h"
#include "../node/MwcNode.h"
#include "../node/MwcNodeConfig.h"
#include "../util/FolderCompressor.h"
#include <QCoreApplication>
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/u_nodeInfo_b.h"
#include <QDir>

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

NodeInfo::NodeInfo(StateContext * _context) :
        State(_context, STATE::NODE_INFO )
{
    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &NodeInfo::onNodeStatus, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onLoginResult,
                     this, &NodeInfo::onLoginResult, Qt::QueuedConnection);

    QObject::connect(context->mwcNode, &node::MwcNode::onMwcStatusUpdate,
                     this, &NodeInfo::onMwcStatusUpdate, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onSubmitFile,
                     this, &NodeInfo::onSubmitFile, Qt::QueuedConnection);

    // Checking/update node status every 20 seconds...
    startTimer(3000); // Let's update node info every 60 seconds. By some reasons it is slow operation...
}

NodeInfo::~NodeInfo() {

}

QString NodeInfo::getMwcNodeStatus() {
    return context->mwcNode->getMwcStatus();
}

NextStateRespond NodeInfo::execute() {
    if ( context->appContext->getActiveWndState() != STATE::NODE_INFO )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if ( state::getStateMachine()->getCurrentStateId() != STATE::NODE_INFO ) {
        core::getWndManager()->pageNodeInfo();
        for (auto b : bridge::getBridgeManager()->getNodeInfo())
            b->setNodeStatus( lastLocalNodeStatus,  lastNodeStatus );
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

node::MwcNode * NodeInfo::getMwcNode() const {
    return context->mwcNode;
}

// After login - let's check the node status
void NodeInfo::onLoginResult(bool ok) {
    if (ok) {
        currentNodeConnection = getNodeConnection();
        lastLocalNodeStatus = "Waiting";
        requestNodeInfo();
        justLogin = true;
    }
}


void NodeInfo::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event)

    timerCounter++;

    // Don't request for init or lock states.
    if ( context->stateMachine->getCurrentStateId() >= STATE::ACCOUNTS ) {
        int div = 1;
        if ( currentNodeConnection.connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD ) {
            // timer is 3 seconds.
            div = 20; // want to update once in a minute
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

wallet::MwcNodeConnection NodeInfo::getNodeConnection() const {
    return context->appContext->getNodeConnection( context->wallet->getWalletConfig().getNetwork() );
}

void NodeInfo::updateNodeConnection( const wallet::MwcNodeConnection & nodeConnect) {
    auto walletConfig = context->wallet->getWalletConfig();
    context->appContext->updateMwcNodeConnection( walletConfig.getNetwork(), nodeConnect );
    context->wallet->setWalletConfig( walletConfig, false );
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
                        "Wallet restore connection to MWC Node");
            }
            else {
                if ( ! notify::notificationStateCheck( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA) )
                    notify::appendNotificationMessage(notify::MESSAGE_LEVEL::CRITICAL,
                                                           "Wallet lost connection to MWC Node");
            }
        }
        else if ( (connections==0) ^ (lastNodeStatus.connections==0) ) {
                if (connections>0) {
                    notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                               "MWC Node restored connection to MWC network");
                }
                else {
                    notify::appendNotificationMessage(notify::MESSAGE_LEVEL::CRITICAL,
                                                               "MWC Node lost connection to MWC network");
                }
        }
        else if ( (nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < peerHeight) ^ (lastNodeStatus.nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < lastNodeStatus.peerHeight) ) {
            if (nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < peerHeight) {
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::CRITICAL,
                                                           "MWC Node out of sync from mwc network");
            }
            else {
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                           "MWC Node finish syncing and runs well now");
            }
        }
    }

    lastNodeStatus.setData(online, errMsg, nodeHeight, peerHeight, totalDifficulty, connections);

    if (justLogin) {
        justLogin = false;
        // Let's consider 5 blocks (5 minutes) unsync be critical issue
        if ( !online || nodeHeight < peerHeight - mwc::NODE_HEIGHT_DIFF_LIMIT || connections==0 ) {
            if ( state::getStateMachine()->getCurrentStateId() != STATE::NODE_INFO) {
                // Switching to this Node Info state. State switch will take care about the rest workflow
                context->stateMachine->setActionWindow( state::STATE::NODE_INFO );
                return;
            }
        }
    }

    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->setNodeStatus( lastLocalNodeStatus, lastNodeStatus );
}



void NodeInfo::onMwcStatusUpdate( QString status ) {
    lastLocalNodeStatus = status;
    logger::logInfo("NodeInfo", "embedded mwc-node status: " + status);
    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->updateEmbeddedMwcNodeStatus(getMwcNodeStatus());
}

QString NodeInfo::getBlockchainDataPath() const {
    return context->appContext->getPathFor("BlockchainData");
}

void    NodeInfo::updateBlockchainDataPath(QString path) {
    context->appContext->updatePathFor("BlockchainData", path);
}

QString NodeInfo::getPublishTransactionPath() const {
    return context->appContext->getPathFor("PublishTransaction");
}
void    NodeInfo::updatePublishTransactionPath(QString path) {
    context->appContext->updatePathFor("PublishTransaction", path);
}

void NodeInfo::exportBlockchainData(QString fileName) {
    // 1. stop the mwc node
    // 2. Export node data
    // 3. start mwc-node

    QCoreApplication::processEvents();

    notify::notificationStateSet( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    context->mwcNode->stop();

    QString network = context->mwcNode->getCurrentNetwork();

    Q_ASSERT(currentNodeConnection.isLocalNode());
    QPair<bool,QString> nodePath = node::getMwcNodePath( currentNodeConnection.localNodeDataPath, network);
    if (!nodePath.first) {
        core::getWndManager()->messageTextDlg("Error", nodePath.second);
        return;
    }

    QCoreApplication::processEvents();

    QPair<bool, QString> res = compress::compressFolder( nodePath.second + "chain_data/", fileName, network );


    QCoreApplication::processEvents();

    context->mwcNode->start(currentNodeConnection.localNodeDataPath, network);

    QCoreApplication::processEvents();

    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->hideProgress();

    QCoreApplication::processEvents();

    notify::notificationStateClean( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    if (res.first) {
        core::getWndManager()->messageTextDlg("MWC Blockchain data is ready", "MWC blockchain data was successfully exported to the archive " + fileName);
    }
    else {
        core::getWndManager()->messageTextDlg("Failed to export MWC Blockchain data", "Unable to export the blockchain data. Error:\n" + res.second);
    }
}

void NodeInfo::importBlockchainData(QString fileName) {
    // 1. stop the mwc node
    // 2. Import node data
    // 3. start mwc-node

    QCoreApplication::processEvents();

    notify::notificationStateSet( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    context->mwcNode->stop();

    Q_ASSERT(currentNodeConnection.isLocalNode());
    QString network = context->mwcNode->getCurrentNetwork();
    QPair<bool,QString> nodePath = node::getMwcNodePath(currentNodeConnection.localNodeDataPath, network);
    if (!nodePath.first) {
        core::getWndManager()->messageTextDlg("Error", nodePath.second);
        return;
    }

    QCoreApplication::processEvents();

    QPair<bool, QString> res = compress::decompressFolder( fileName,  nodePath.second + "chain_data/", network );

    QCoreApplication::processEvents();

    context->mwcNode->start(currentNodeConnection.localNodeDataPath, network);

    QCoreApplication::processEvents();

    notify::notificationStateClean( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->hideProgress();

    QCoreApplication::processEvents();

    if (res.first) {
        core::getWndManager()->messageTextDlg("MWC Blockchain data is ready", "MWC blockchain data was successfully imported from the archive " + fileName);
    }
    else {
        core::getWndManager()->messageTextDlg("Failed to import MWC Blockchain data", "Unable to import the blockchain data. Error:\n" + res.second);
    }
}

void NodeInfo::publishTransaction(QString fileName) {
    //   dssfddf
    context->wallet->submitFile(fileName);
    // Respond at onSubmitFile
}

void NodeInfo::onSubmitFile(bool success, QString message, QString fileName) {
    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->hideProgress();

    if (success) {
        core::getWndManager()->messageTextDlg("Transaction published", "You transaction at " + fileName +
        " was successfully delivered to your local node. Please keep your node running for some time to deliver it to MWC blockchain.\n" + message);
    }
    else {
        if (message.contains("Post TX Error: Request error: Wrong response code: 500 Internal Server Error with data Body(Streaming)"))
            message = "MWC Node unable to publish this transaction. Probably this transaction is already published or original output doesn't exist any more";

        core::getWndManager()->messageTextDlg("Transaction failed", "Transaction from " + fileName +
                  " was not delivered to your local node.\n\n" + message);
    }
}


void NodeInfo::resetEmbeddedNodeData() {
    notify::notificationStateSet( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    Q_ASSERT(currentNodeConnection.isLocalNode());
    QString network = context->mwcNode->getCurrentNetwork();
    QPair<bool,QString> nodePath = node::getMwcNodePath(currentNodeConnection.localNodeDataPath, network);
    if (!nodePath.first) {
        core::getWndManager()->messageTextDlg("Error", nodePath.second);
        return;
    }

    context->mwcNode->stop();

    QCoreApplication::processEvents();

    // Cleaning up the folder
    QString nodeDataPath = nodePath.second + "chain_data/";
    QDir dir(nodeDataPath);
    if (!dir.removeRecursively()) {
        core::getWndManager()->messageTextDlg("Error", "Unable to clean up the node data at " + nodeDataPath);
    }

    QCoreApplication::processEvents();

    context->mwcNode->start(currentNodeConnection.localNodeDataPath, network);

    notify::notificationStateClean( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->hideProgress();

    QCoreApplication::processEvents();
}


}

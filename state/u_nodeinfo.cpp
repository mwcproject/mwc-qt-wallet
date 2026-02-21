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
#include <QtConcurrent>

#include "zz_heart_beat.h"
#include "node/node_client.h"

#include "util/message_mapper.h"

namespace state {

////////////////////////////////////////////////////

NodeInfo::NodeInfo(StateContext * _context) :
        State(_context, STATE::NODE_INFO )
{
}

NodeInfo::~NodeInfo() {
    runningJob.waitForFinished();
}

NextStateRespond NodeInfo::execute() {
    logger::logInfo(logger::STATE, "Call NodeInfo::execute");
    if ( context->appContext->getActiveWndState() != STATE::NODE_INFO )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if ( state::getStateMachine()->getCurrentStateId() != STATE::NODE_INFO ) {
        core::getWndManager()->pageNodeInfo();

        HeartBeat * hbState = (HeartBeat *) context->stateMachine->getState( STATE::HEART_BEAT );
        hbState->updateNodeStatus();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

/*node::MwcNode * NodeInfo::getMwcNode() const {
    return context->mwcNode;
}*/

bool NodeInfo::isNodeHealthy() const {
    logger::logInfo(logger::STATE, "Call NodeInfo::isNodeHealthy");
    return context->wallet->isNodeHealthy();
}


void NodeInfo::requestWalletResync() {
    logger::logInfo(logger::STATE, "Call NodeInfo::requestWalletResync");
    context->appContext->pushCookie("PrevState", (int)context->appContext->getActiveWndState() );
    context->stateMachine->setActionWindow( state::STATE::RESYNC );
}

void NodeInfo::exportBlockchainData(QString fileName) {
    logger::logInfo(logger::STATE, "Call NodeInfo::exportBlockchainData with fileName=" + fileName);
    // 1. stop the mwc node
    // 2. Export node data
    // 3. start mwc-node

    notify::notificationStateSet( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    std::shared_ptr<node::NodeClient> nodeClient = context->wallet->getNodeClient();
    NodeInfo * response = this;
    runningJob.waitForFinished();

    runningJob = QtConcurrent::run( [nodeClient, fileName, response]() {
        std::shared_ptr<node::MwcNode> mwcNode = nodeClient->takeEmbeddedNode();
        Q_ASSERT(mwcNode!=nullptr);
        QPair<bool, QString> res;
        if (mwcNode==nullptr) {
            res = QPair<bool, QString>(false, "Mwc Node is not running");
        }
        else {
            QString nodeNetwork = mwcNode->getCurrentNetwork();
            QString nodePath = mwcNode->getNodeDataPath();

            mwcNode->stop();

            res = compress::compressFolder( nodePath, fileName, nodeNetwork );

            mwcNode->start(nodePath, nodeNetwork); //, context->appContext->useTorForNode());
            nodeClient->restoreEmbeddedNode(mwcNode);
        }

        QMetaObject::invokeMethod(response,
        [response, res, fileName]() {
                response->finishExportBlockchainData(res, fileName);
            },
            Qt::QueuedConnection);
    });
}

void NodeInfo::finishExportBlockchainData(QPair<bool, QString> res, QString fileName) {
    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->hideProgress();

    notify::notificationStateClean( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    if (res.first) {
        core::getWndManager()->messageTextDlg("MWC Blockchain data is ready", "MWC blockchain data was successfully exported to the archive " + fileName);
    }
    else {
        core::getWndManager()->messageTextDlg("Failed to export MWC Blockchain data", "Unable to export the blockchain data. Error:\n" + res.second);
    }
}

void NodeInfo::importBlockchainData(QString fileName) {
    logger::logInfo(logger::STATE, "Call NodeInfo::importBlockchainData with fileName=" + fileName);
    // 1. stop the mwc node
    // 2. Import node data
    // 3. start mwc-node

    notify::notificationStateSet( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    std::shared_ptr<node::NodeClient> nodeClient = context->wallet->getNodeClient();
    NodeInfo * response = this;
    runningJob.waitForFinished();

    runningJob = QtConcurrent::run( [nodeClient, fileName, response]() {
        std::shared_ptr<node::MwcNode> mwcNode = nodeClient->takeEmbeddedNode();
        Q_ASSERT(mwcNode!=nullptr);
        QPair<bool, QString> res;
        if (mwcNode==nullptr) {
            res = QPair<bool, QString>(false, "Mwc Node is not running");
        }
        else {
            QString nodeNetwork = mwcNode->getCurrentNetwork();
            QString nodePath = mwcNode->getNodeDataPath();

            mwcNode->stop();

            res = compress::decompressFolder( fileName,  nodePath, nodeNetwork );

            mwcNode->start(nodePath, nodeNetwork); //, context->appContext->useTorForNode());
            nodeClient->restoreEmbeddedNode(mwcNode);
        }

        QMetaObject::invokeMethod(response,
        [response, res, fileName]() {
                response->finishImportBlockchainData(res, fileName);
            },
            Qt::QueuedConnection);
    });
}

void NodeInfo::finishImportBlockchainData(QPair<bool, QString> res, QString fileName) {
    notify::notificationStateClean( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->hideProgress();

    if (res.first) {
        core::getWndManager()->messageTextDlg("MWC Blockchain data is ready", "MWC blockchain data was successfully imported from the archive " + fileName);
    }
    else {
        core::getWndManager()->messageTextDlg("Failed to import MWC Blockchain data", "Unable to import the blockchain data. Error:\n" + res.second);
    }
}

void NodeInfo::publishTransaction(QString fileName, bool fluff) {
    logger::logInfo(logger::STATE, "Call NodeInfo::publishTransaction with fileName=" + fileName + " fluff=" + (fluff ? "true" : "false"));
    //   dssfddf
    QString error = context->wallet->submitFile(fileName, fluff);

    if (error.isEmpty()) {
        core::getWndManager()->messageTextDlg("Transaction published", "You transaction at " + fileName +
        " was successfully delivered to your local node. Please keep your node running for some time to deliver it to MWC blockchain.");
    }
    else {
        if (error.contains("Post TX Error: Request error: Wrong response code: 500 Internal Server Error with data Body(Streaming)"))
            error = "MWC Node unable to publish this transaction. Probably this transaction is already published or original output doesn't exist any more";

        core::getWndManager()->messageTextDlg("Transaction failed", "Transaction from " + fileName +
                  " was not delivered to your local node.\n\n" + util::mapMessage(error));
    }
}


void NodeInfo::resetEmbeddedNodeData() {
    logger::logInfo(logger::STATE, "Call NodeInfo::resetEmbeddedNodeData");
    notify::notificationStateSet( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    std::shared_ptr<node::NodeClient> nodeClient = context->wallet->getNodeClient();
    NodeInfo * response = this;
    runningJob.waitForFinished();

    runningJob = QtConcurrent::run( [nodeClient, response]() {
        std::shared_ptr<node::MwcNode> mwcNode = nodeClient->takeEmbeddedNode();
        Q_ASSERT(mwcNode!=nullptr);
        QString errMessage;
        if (mwcNode!=nullptr) {
            QString nodeNetwork = mwcNode->getCurrentNetwork();
            QString nodePath = mwcNode->getNodeDataPath();

            mwcNode->stop();

            // Cleaning up the folder
            QString nodeDataPath = nodePath;
            QDir dir(nodeDataPath);
            if (!dir.removeRecursively()) {
                errMessage = "Unable to clean up the node data at " + nodeDataPath;
            }

            mwcNode->start(nodePath, nodeNetwork); //, context->appContext->useTorForNode());
            nodeClient->restoreEmbeddedNode(mwcNode);
        }
        else {
            errMessage = "Mwc Node is not running";
        }

        QMetaObject::invokeMethod(response,
        [response, errMessage]() {
                response->finishResetEmbeddedNodeData(errMessage);
            },
            Qt::QueuedConnection);
    });
}

void NodeInfo::finishResetEmbeddedNodeData(QString errorMessage) {
    notify::notificationStateClean( notify::NOTIFICATION_STATES::ONLINE_NODE_IMPORT_EXPORT_DATA );

    for (auto b : bridge::getBridgeManager()->getNodeInfo())
        b->hideProgress();

    if (!errorMessage.isEmpty()) {
        core::getWndManager()->messageTextDlg("Error", errorMessage);
    }
}

}

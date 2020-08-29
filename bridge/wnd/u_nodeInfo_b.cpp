// Copyright 2020 The MWC Developers
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

#include "u_nodeInfo_b.h"
#include "../BridgeManager.h"
#include "../../state/state.h"
#include "../../state/u_nodeinfo.h"

namespace bridge {

static state::NodeInfo * getState() {return (state::NodeInfo *) getState(state::STATE::NODE_INFO); }

NodeInfo::NodeInfo(QObject * parent) : QObject(parent) {
    getBridgeManager()->addNodeInfo(this);
}

NodeInfo::~NodeInfo() {
    getBridgeManager()->removeNodeInfo(this);
}

void NodeInfo::setNodeStatus( const QString & localNodeStatus, const state::NodeStatus & status ) {
    emit sgnSetNodeStatus( localNodeStatus,
            status.online,  status.errMsg, status.nodeHeight, status.peerHeight,
            util::longLong2ShortStr(status.totalDifficulty, 9), status.connections);
}

void NodeInfo::updateEmbeddedMwcNodeStatus( const QString & status ) {
    emit sgnUpdateEmbeddedMwcNodeStatus(status);
}
void NodeInfo::hideProgress() {
    emit sgnHideProgress();
}

// request wallet::MwcNodeConnection as a Json
QString NodeInfo::getNodeConnection() {
    return getState()->getNodeConnection().toJson();
}

// mwc Node status string
QString NodeInfo::getMwcNodeStatus() {
    return getState()->getMwcNodeStatus();
}

// Request wallet full resync
void NodeInfo::requestWalletResync() {
    getState()->requestWalletResync();
}

// Update node connection data for the wallet. Might require relogin
void NodeInfo::updateNodeConnection(QString mwcNodeConnectionJson) {
    getState()->updateNodeConnection( wallet::MwcNodeConnection::fromJson(mwcNodeConnectionJson) );
}

// Export blockchain data to the archive
void NodeInfo::exportBlockchainData(QString fileName) {
    getState()->exportBlockchainData(fileName);
}
// Import blockchain data from the archive
void NodeInfo::importBlockchainData(QString fileName) {
    getState()->importBlockchainData(fileName);
}
// publish transaction from the file
void NodeInfo::publishTransaction(QString fileName) {
    getState()->publishTransaction(fileName);
}

// Reset embedded node data
void NodeInfo::resetEmbeddedNodeData() {
    getState()->resetEmbeddedNodeData();
}


}

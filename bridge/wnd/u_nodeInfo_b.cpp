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
#include "../../state/zz_heart_beat.h"
#include "../../util/Log.h"

namespace bridge {

static state::NodeInfo * getState() {return (state::NodeInfo *) getState(state::STATE::NODE_INFO); }
static state::HeartBeat * getHeartBeatState() {return (state::HeartBeat *) getState(state::STATE::HEART_BEAT); }

NodeInfo::NodeInfo(QObject * parent) : QObject(parent) {
    getBridgeManager()->addNodeInfo(this);
}

NodeInfo::~NodeInfo() {
    getBridgeManager()->removeNodeInfo(this);
}

void NodeInfo::hideProgress() {
    emit sgnHideProgress();
}


// mwc Node status string
void NodeInfo::requestMwcNodeStatus() {
    logger::logInfo(logger::BRIDGE, "Call NodeInfo::requestMwcNodeStatus");
    return getHeartBeatState()->updateNodeStatus();
}

// Request wallet full resync
void NodeInfo::requestWalletResync() {
    logger::logInfo(logger::BRIDGE, "Call NodeInfo::requestWalletResync");
    getState()->requestWalletResync();
}

// Export blockchain data to the archive
void NodeInfo::exportBlockchainData(QString fileName) {
    logger::logInfo(logger::BRIDGE, "Call NodeInfo::exportBlockchainData with fileName=" + fileName);
    getState()->exportBlockchainData(fileName);
}
// Import blockchain data from the archive
void NodeInfo::importBlockchainData(QString fileName) {
    logger::logInfo(logger::BRIDGE, "Call NodeInfo::importBlockchainData with fileName=" + fileName);
    getState()->importBlockchainData(fileName);
}
// publish transaction from the file
void NodeInfo::publishTransaction(QString fileName, bool fluff) {
    logger::logInfo(logger::BRIDGE, "Call NodeInfo::publishTransaction with fileName=" + fileName + " fluff=" + (fluff ? "true" : "false"));
    getState()->publishTransaction(fileName, fluff);
}

// Reset embedded node data
void NodeInfo::resetEmbeddedNodeData() {
    logger::logInfo(logger::BRIDGE, "Call NodeInfo::resetEmbeddedNodeData");
    getState()->resetEmbeddedNodeData();
}


}

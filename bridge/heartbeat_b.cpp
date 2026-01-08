// Copyright 2025 The MWC Developers
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

#include "heartbeat_b.h"
#include "BridgeManager.h"
#include "util/stringutils.h"
#include "../wallet/wallet_objs.h"

namespace bridge {

HeartBeat::HeartBeat(QObject *parent) : QObject(parent) {
    getBridgeManager()->addHeartBeat(this);
}

HeartBeat::~HeartBeat() {
    getBridgeManager()->removeHeartBeat(this);
}

void HeartBeat::emitUpdateListenerStatus(bool mwcOnline, bool torOnline) {
    emit sgnUpdateListenerStatus(mwcOnline, torOnline);
}

void HeartBeat::emitNodeStatus( const QString & localNodeStatus, const wallet::NodeStatus & status ) {
    emit sgnSetNodeStatus( localNodeStatus,
            status.internalNode,
            status.online,  status.nodeHeight, status.peerHeight,
            util::longLong2ShortStr(status.totalDifficulty, 9), status.connections);
}


}

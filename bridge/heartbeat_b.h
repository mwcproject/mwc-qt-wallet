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

#ifndef MWC_QT_WALLET_HEARTBEAT_B_H
#define MWC_QT_WALLET_HEARTBEAT_B_H

#include <QObject>

namespace wallet {
    struct NodeStatus;
}

namespace bridge {

class HeartBeat : public QObject {
    Q_OBJECT
public:
    explicit HeartBeat(QObject *parent = nullptr);
    ~HeartBeat();

    void emitNodeStatus( const QString & localNodeStatus, const wallet::NodeStatus & status );

    void emitUpdateListenerStatus(bool mwcOnline, bool torOnline);
signals:
    void sgnUpdateListenerStatus(bool mwcOnline, bool torOnline);

    void sgnSetNodeStatus( QString embeddedNodeStatus,
                         bool internalNode,
                         bool online,  int nodeHeight, int peerHeight,
                         QString totalDifficulty2show, int connections);

private:
};

}


#endif //MWC_QT_WALLET_HEARTBEAT_B_H
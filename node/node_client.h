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


#ifndef MWC_QT_WALLET_NODE_CLIENT_H
#define MWC_QT_WALLET_NODE_CLIENT_H
#include <QObject>
#include <QString>

#include "wallet/wallet_objs.h"

namespace util {
    class HttpClient;
}

namespace node {
    class MwcNode;

class NodeClient  : public QObject {
    Q_OBJECT
public:
    NodeClient(QString network, node::MwcNode * embeddedNode);
    virtual ~NodeClient();

    QString foreignApiRequest(const QString & request);

    wallet::NodeStatus requestNodeStatus();

    qint64 getLastNodeHeight() const {return lastNodeHeight;}
    const QString & getLastInternalNodeState() const {return lastInternalNodeState;}

    bool isNodeHealthy();

    bool isUsePublicNode() const {return usePublicNode;}
private:
    signals:

private:
    void updateNodeSelection(int retry=3);

    QString callPublicNodeForeignApi(const QString & request);
private:
    QString network;
    node::MwcNode * embeddedNode = nullptr;
    // Client can use public or embedded node if it is synced up.
    volatile bool usePublicNode = true;
    int publicNodeIdx;
    // check time to define if internal or public node will be used
    qint64 nodeStatusCheckTime = 0;

    util::HttpClient * publicNodeClient = nullptr;

    qint64 lastNodeHeight = 0;

    wallet::NodeStatus lastStatus;
    QString lastInternalNodeState;
    qint64 lastStatusTime = 0;
};

void updatePablicNodeLatency();

}


#endif //MWC_QT_WALLET_NODE_CLIENT_H

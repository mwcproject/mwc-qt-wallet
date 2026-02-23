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
#include <QMutex>

#include "MwcNode.h"
#include "wallet/wallet_objs.h"

namespace wallet {
    class Wallet;
}

namespace util {
    class HttpClient;
}

namespace node {
    class MwcNode;

class NodeClient {
public:
    NodeClient(const QString & nodeDataPath, QString network, core::AppContext * appContext, QFuture<QString> * torStarter);
    virtual ~NodeClient();

    QString foreignApiRequest(const QString & request);

    wallet::NodeStatus requestNodeStatus();

    qint64 getLastNodeHeight() const {return lastNodeHeight;}
    QString getLastInternalNodeState() const;

    bool isNodeAlive() const;
    bool isNodeHealthy();

    bool isUsePublicNode() const {return usePublicNode;}

    std::shared_ptr<node::MwcNode> takeEmbeddedNode();
    void restoreEmbeddedNode(std::shared_ptr<node::MwcNode> node);

    QString getCurrentNetwork() const;
private:
    signals:

private:
    void updateNodeSelectionLocked(int retry=3);

    QString callPublicNodeForeignApiLocked(const QString & request);

    wallet::NodeStatus requestNodeStatusLocked();
private:
    // Don't call multiple requests with the same client.
    mutable QMutex callMutex;

    const QString network;
    std::shared_ptr<node::MwcNode> embeddedNode;
    // Client can use public or embedded node if it is synced up.
    bool usePublicNode = true;
    int publicNodeIdx = 0;
    // check time to define if internal or public node will be used
    qint64 nodeStatusCheckTime = 0;

    util::HttpClient * publicNodeClient = nullptr;

    volatile qint64 lastNodeHeight = 0;

    wallet::NodeStatus lastStatus;
    QString lastInternalNodeState;
    qint64 lastStatusTime = 0;
};

void updatePablicNodeLatency();

}


#endif //MWC_QT_WALLET_NODE_CLIENT_H

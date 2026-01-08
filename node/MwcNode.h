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

#ifndef MWC_QT_WALLET_MWCNODE_H
#define MWC_QT_WALLET_MWCNODE_H

#include <QObject>
#include <QProcess>
#include <QVector>
#include "MwcNodeApi.h"
#include "wallet/tasks/TorEmbeddedStarter.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace core {
class AppContext;
}

namespace wallet {
class Wallet;
}

namespace node {

struct PeerInfo {
    QString address;
    int     totalHeight;
};

struct PeerConnectionInfo {
    QVector<PeerInfo>   peerConnections;
    int64_t             updateTime       = 0;
};

struct NodeStatus {
    int connections     = 0;
    int tipHeight       = 0;
    int64_t updateTime  = 0;
};

enum class SYNC_STATE {GETTING_HEADERS, GETTING_PIBD, VERIFY_KERNELS_HISTORY, VERIFY_HEADERS, VERIFY_KERNEKLS_POS, VERIFY_RANGEPROOFS, VERIFY_KERNEL, GETTING_BLOCKS };

class MwcNode;

// mwc-node lifecycle management
class MwcNode : public QObject {
Q_OBJECT
public:
    // nodePath - path to the executable
    MwcNode(core::AppContext * appContext, QFuture<QString> * torStarter);
    virtual ~MwcNode() override;

    bool isRunning() const {return nodeContextId>=0 || startingNode.isValid();}
    const QString & getCurrentNetwork() const { return nodeNetwork; }
    const QString & getNodeDataPath() const {return nodeDataPath;}

    void setNodeContextId(int id) {nodeContextId = id;}

    // Tor will be started only if it is not running yet
    void start( const QString & nodeDataPath, const QString & network );
    void stop();

    // Return status And progress String
    QPair<ServerStats, QString> getServerStats();

    QString callForiegnApi(const QString & request);

    void reportNodeFatalError( QString message );

    void submitStartNodeResult(QString errorStr, int context_id, bool isStartCancelled);
private:
    void waitCancelStart();

private:
    QString calcProgressStr( bool pibdSync, SYNC_STATE syncState, int minVal, int cur, int max );

private slots:
private:
    core::AppContext *appContext; // app context to store current account name
    QFuture<QString> * torStarter;

    int nodeContextId = -1;
    QString nodeDataPath;
    QString nodeNetwork;
    QAtomicInt stoppingFlag;
    QFuture<void> startingNode;
    bool pibdSyncPhase = false;
};

QString calculateRunningStatus(bool & pibdSyncPhase, ServerStats stats);

}

#endif //MWC_QT_WALLET_MWCNODE_H

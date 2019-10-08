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
#include "../tries/NodeOutputParser.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace core {
class AppContext;
}

namespace tries{
class NodeOutputParser;
}

namespace node {

// Node management timeouts.
const int64_t CHECK_NODE_PERIOD = 60 * 1000; // Timer check period. API calls to node will be issued
const int64_t API_FAILURE_LIMIT = 5; // Restart node anter not happy API calls. Note, in cold storage that will be case
const int64_t START_TIMEOUT   = 90*1000;
// messages from NodeOutputParser
const int64_t MWC_NODE_STARTED_TIMEOUT = 180*1000; // It can take some time to find peers
const int64_t MWC_NODE_SYNC_MESSAGES = 60*1000; // Sync supposed to be agile
const int64_t RECEIVE_BLOCK_LISTEN = 10*60*1000; // 10 minutes can be delay due non consistancy. API call expected to catch non sync cases
const int64_t NETWORK_ISSUES = 0; // Let's not consider network issues. API call will restart the node

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

// mwc-node lifecycle management
class MwcNode : public QObject {
Q_OBJECT
public:
    MwcNode(QString nodePath, core::AppContext * appContext);
    virtual ~MwcNode() override;

    bool isRunning() const {return nodeProcess!= nullptr;}
    const QString & getCurrentNetwork() const { return lastUsedNetwork; }

    void start( const QString & network );
    void stop();

private:
    QProcess * initNodeProcess( QString network );

    void nodeProcDisconnect();
    void nodeProcConnect(QProcess * process);

    void sendRequest( const QString & tag, QString secret, const QString & api);

    QString getNodeSecret();
private:
    virtual void timerEvent(QTimerEvent *event) override;

private slots:
    void nodeErrorOccurred(QProcess::ProcessError error);
    void nodeProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void mwcNodeReadyReadStandardError();
    void mwcNodeReadyReadStandardOutput();

    void replyFinished(QNetworkReply* reply);

    void nodeOutputGenericEvent( tries::NODE_OUTPUT_EVENT event, QString message);

private:
    core::AppContext *appContext; // app context to store current account name

    QString nodePath; // path to the backed binary
    QProcess *nodeProcess = nullptr;
    tries::NodeOutputParser *nodeOutputParser = nullptr; // logs will come from stdout

    QString lastUsedNetwork;
    PeerConnectionInfo peers; // connected peers. Polling with API
    NodeStatus         nodeStatus;
    QString nodeSecret;
    QString nodeWorkDir;

    int64_t respondTimelimit = 0; // Get some respond from node. Will be happy until that time.

    QVector< QMetaObject::Connection > processConnections; // open connection to mwc713

    int nodeCheckFailCounter = 0;
    int nodeOutOfSyncCounter = 0;
    int lastKnownHeight = 0;

    QNetworkAccessManager *nwManager;

    tries::NODE_OUTPUT_EVENT lastProcessedEvent = tries::NODE_OUTPUT_EVENT::NONE;
};

}

#endif //MWC_QT_WALLET_MWCNODE_H

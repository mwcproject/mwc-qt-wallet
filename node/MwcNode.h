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
const int64_t CHECK_NODE_PERIOD = 5 * 1000; // Timer check period. API calls to node will be issued
const int64_t NODE_OUT_OF_SYNC_FAILURE_LIMIT = 60; // Node ouf of sync and nothing was updated...
const int64_t NODE_NO_PEERS_FAILURE_LIMITS = 60; // Let's wait 5 minutes before restarts

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
    // nodePath - path to the executable
    MwcNode(const QString & nodePath, core::AppContext * appContext);
    virtual ~MwcNode() override;

    bool isRunning() const {return nodeProcess!= nullptr;}
    const QString & getCurrentNetwork() const { return lastUsedNetwork; }

    void start( const QString & dataPath, const QString & network );
    void stop();

    QString getMwcStatus() const { return nodeStatusString; }

    // Last Many node output lines. There are many of them.
    // Call from the same thread
    const QStringList & getOutputLines() const {return outputLines;}

    QString getLogsLocation() const;
private:
    QProcess * initNodeProcess( const QString & dataPath, const QString & network );

    void nodeProcDisconnect();
    void nodeProcConnect(QProcess * process);

    enum class REQUEST_TYPE { GET, POST };
    void sendRequest( const QString & tag, QString secret, const QString & api, REQUEST_TYPE reqType = REQUEST_TYPE::GET);

    QString getNodeSecret();

    void reportNodeFatalError( QString message );

    void updateRunningStatus();
private:
    virtual void timerEvent(QTimerEvent *event) override;

private: signals:
    void onMwcOutputLine(QString line);
    void onMwcStatusUpdate(QString status);

private slots:
    void nodeErrorOccurred(QProcess::ProcessError error);
    void nodeProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void mwcNodeReadyReadStandardError();
    void mwcNodeReadyReadStandardOutput();

    void replyFinished(QNetworkReply* reply);

    void nodeOutputGenericEvent( tries::NODE_OUTPUT_EVENT event, QString message);

    // One short timer to restart the node. Usinng instead of sleep
    void onRestartNode();
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

    int nodeNoPeersFailCounter = 0;
    int nodeOutOfSyncCounter = 0;
    int nodeHeight = 0;
    int peersMaxHeight = 0;
    int initChainHeight = 0;

    QNetworkAccessManager *nwManager;

    tries::NODE_OUTPUT_EVENT lastProcessedEvent = tries::NODE_OUTPUT_EVENT::NONE;

    QString nonEmittedOutput;

    QString nodeStatusString;
    int     txhashsetHeight = 0;
    int     maxBlockHeight = 0; // backing stopper for getted blocks.
    bool    syncIsDone = false;

    // Last Many node output lines
    QStringList outputLines;

    // Will try to restart the node several times.
    // The reason that because of another instance is running
    int restartCounter = 0;

    QString lastDataPath;
};

}

#endif //MWC_QT_WALLET_MWCNODE_H

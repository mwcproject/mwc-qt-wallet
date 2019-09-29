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

namespace core {
class AppContext;
}

namespace tries{
class NodeLogsParser;
}

namespace node {

struct PeerInfo {
    QString address;
    int     totalHeight;
};

struct PeerConnectionInfo {
    QVector<PeerInfo>   peerConnections;
    int64_t             updateTime;
};

struct NodeStatus {
    int connections = 0;
    int tipHeight      = 0;
    int64_t             updateTime;
};

// mwc-node lifecycle management
class MwcNode : public QObject {
Q_OBJECT
public:
    MwcNode(QString nodePath, QString nodeConfigPath, core::AppContext * appContext);


private:
    core::AppContext *appContext; // app context to store current account name

    QString mwc713Path; // path to the backed binary
    QString mwc713configPath; // config file for mwc713
    QProcess *mwc713process = nullptr;
    tries::NodeLogsParser *logsParser = nullptr; // logs will come from stdout

    PeerConnectionInfo peers; // connected peers. Polling with API
    NodeStatus         nodeStatus;

    int64_t respondTimelimit; // Get some respond from node. Will be happy until that time.

};

}

#endif //MWC_QT_WALLET_MWCNODE_H

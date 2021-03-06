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

#ifndef MWC_QT_WALLET_NODEINFO_H
#define MWC_QT_WALLET_NODEINFO_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../node/MwcNodeConfig.h"

namespace state {

struct NodeStatus {
    bool online = false;
    QString errMsg;
    int nodeHeight = 0;
    int peerHeight = 0;
    int64_t totalDifficulty = 0;
    int connections = 0;

    void setData(bool online,
            const QString & errMsg,
            int nodeHeight,
            int peerHeight,
            int64_t totalDifficulty,
            int connections);
};


class NodeInfo : public QObject, public State {
Q_OBJECT
public:
    NodeInfo(StateContext * context);
    virtual ~NodeInfo() override;

    void requestNodeInfo();

    void requestWalletResync();

    int getCurrentNodeHeight() {return lastNodeStatus.nodeHeight;}

    // Connection with network
    wallet::MwcNodeConnection getNodeConnection() const;
    void updateNodeConnection( const wallet::MwcNodeConnection & nodeConnect);

    QString getMwcNodeStatus();

    node::MwcNode * getMwcNode() const;

    void exportBlockchainData(QString fileName);
    void importBlockchainData(QString fileName);
    void publishTransaction(QString fileName);
    void resetEmbeddedNodeData();
protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override {return false;}
    virtual QString getHelpDocName() override {return "node_overview.html";}

private slots:
    void onLoginResult(bool ok);

    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

    void onMwcStatusUpdate(QString status);

    void onSubmitFile(bool success, QString message, QString fileName);

private:
    virtual void timerEvent(QTimerEvent *event) override;
private:
    bool  justLogin = false;
    NodeStatus lastNodeStatus; // Satus as mwc713 see the node
    QString lastLocalNodeStatus = "Waiting"; // Status from the embedded node
    int timerCounter = 0; // update is different in different modes.
    wallet::MwcNodeConnection currentNodeConnection;
};

}

#endif //MWC_QT_WALLET_NODEINFO_H

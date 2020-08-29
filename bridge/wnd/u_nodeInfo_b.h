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

#ifndef MWC_QT_WALLET_U_NODEINFO_B_H
#define MWC_QT_WALLET_U_NODEINFO_B_H

#include <QObject>
#include "../../state/u_nodeinfo.h"

namespace bridge {

class NodeInfo : public QObject {
Q_OBJECT
public:
    explicit NodeInfo(QObject * parent = nullptr);
    ~NodeInfo();

    void setNodeStatus( const QString & localNodeStatus, const state::NodeStatus & status );
    void updateEmbeddedMwcNodeStatus( const QString & status );
    void hideProgress();

    // request wallet::MwcNodeConnection as a Json
    Q_INVOKABLE QString getNodeConnection();

    // mwc Node status string
    Q_INVOKABLE QString getMwcNodeStatus();

    // Request wallet full resync
    Q_INVOKABLE void requestWalletResync();

    // Update node connection data for the wallet. Might require relogin
    Q_INVOKABLE void updateNodeConnection(QString mwcNodeConnectionJson);

    // Export blockchain data to the archive
    Q_INVOKABLE void exportBlockchainData(QString fileName);
    // Import blockchain data from the archive
    Q_INVOKABLE void importBlockchainData(QString fileName);
    // publish transaction from the file
    Q_INVOKABLE void publishTransaction(QString fileName);

    // Reset embedded node data
    Q_INVOKABLE void resetEmbeddedNodeData();
signals:
    void sgnSetNodeStatus( QString localNodeStatus,
                             bool online,  QString errMsg, int nodeHeight, int peerHeight,
                            QString totalDifficulty2show, int connections);

    void sgnUpdateEmbeddedMwcNodeStatus( QString status );

    void sgnHideProgress();
};

}

#endif //MWC_QT_WALLET_U_NODEINFO_B_H

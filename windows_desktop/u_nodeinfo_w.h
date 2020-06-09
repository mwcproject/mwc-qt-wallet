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

#ifndef U_NODEINFO_H
#define U_NODEINFO_H

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class NodeInfo;
}

namespace bridge {
class Config;
class Wallet;
class NodeInfo;
class Util;
}

namespace wnd {

class NodeInfo : public core::NavWnd {
Q_OBJECT

public:
    explicit NodeInfo(QWidget *parent);
    ~NodeInfo();

private:
    // Empty string to hide warning...
    void showWarning(QString warning);
    void showNodeLogs();
    void updateNodeReadyButtons(bool nodeIsReady);

private slots:
    void onShowNodeConnectionError(QString errorMessage);

    void on_refreshButton_clicked();
    void on_changeNodeButton_clicked();
    void on_showLogsButton_clicked();
    void on_showLogsButton_5_clicked();
    void on_showLogsButton_8_clicked();
    void on_saveBlockchianData_clicked();
    void on_loadBlockchainData_clicked();
    void on_publishTransaction_clicked();

    void onSgnSetNodeStatus( QString localNodeStatus,
                                     bool online,  QString errMsg, int nodeHeight, int peerHeight,
                                     QString totalDifficulty2show, int connections);
    void onSgnUpdateEmbeddedMwcNodeStatus( QString status );
    void onSgnHideProgress();

private:
signals:
    void showNodeConnectionError(QString errorMessage);

private:
    Ui::NodeInfo *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::NodeInfo * nodeInfo = nullptr;
    bridge::Util * util = nullptr;

    wallet::MwcNodeConnection::NODE_CONNECTION_TYPE connectionType;
    QString currentWarning = "?";

    // Cache for latest error. We don't want spam user with messages about the node connection
    static QString lastShownErrorMessage;
};

}

#endif // U_NODEINFO_H

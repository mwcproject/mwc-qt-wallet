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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "../state/state.h"
#include "../core/Notification.h"

namespace Ui {
class MainWindow;
}

namespace bridge {
    class Config;
    class CoreWindow;
    class Wallet;
    class StateMachine;
    class Util;
}

class QPushButton;

namespace core {

class WindowManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QWidget * getMainWindow();


private slots:
    // Update tb & menu actions
    void onUpdateActionStates(int actionState); // state::STATE

    void onNewNotificationMessage(int level, QString message); // level: notify::MESSAGE_LEVEL
    void onConfigUpdate();

    void updateListenerStatus(bool mwcOnline, bool keybaseOnline, bool tor);
    void onHttpListeningStatus(bool listening, QString additionalInfo);
    void updateNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

    void onUpdateSyncProgress(double progressPercent);

    // Internal UI
    void on_listenerStatusButton_clicked();
    void on_nodeStatusButton_clicked();
    void on_helpButton_clicked();
    void on_actionSend_triggered();
    void on_actionExchanges_triggered();
    void on_actionReceive_triggered();
    void on_actionFinalize_triggered();
    void on_actionTransactions_triggered();
    void on_actionListeners_triggered();
    void on_actionNode_Overview_triggered();
    void on_actionResync_with_full_node_triggered();
    void on_actionOutputs_triggered();
    void on_actionAirdrop_triggered();
    void on_actionHODL_triggered();
    void on_actionWallet_accounts_triggered();
    void on_actionAccounts_triggered();
    void on_actionContacts_triggered();
    void on_actionShow_passphrase_triggered();
    void on_actionEvent_log_triggered();
    void on_actionLogout_triggered();
    void on_actionConfig_triggered();
    void on_actionRunning_Mode_Cold_Wallet_triggered();
    void on_actionBlock_Explorer_triggered();
    void on_actionWhite_papers_triggered();
    void on_actionGood_Money_triggered();
    void on_actionRoadmap_triggered();
    void on_actionMWC_website_triggered();

private:
    void updateLeftBar(bool show);
    void updateListenerBtn();
    void updateNetworkName();

    void updateMenu();

    enum class STATUS { IGNORE, RED, YELLOW, GREEN };
    void setStatusButtonState( QPushButton * btn, STATUS status, QString text );

private:
    Ui::MainWindow *ui;
    bridge::Config * config = nullptr;
    bridge::CoreWindow * coreWindow = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::StateMachine * stateMachine = nullptr;
    bridge::Util * util = nullptr;
    bool leftBarShown = true;
};

}

#endif // MAINWINDOW_H

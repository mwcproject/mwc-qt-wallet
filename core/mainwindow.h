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
#include "../wallet/wallet.h"
#include "../core/Notification.h"
#include "appcontext.h"

namespace Ui {
class MainWindow;
}

namespace state {
    class StateMachine;
}


#ifdef Q_OS_DARWIN
namespace Cocoa
{
    void changeTitleBarColor(WId winId, double red, double green, double blue);
}
#endif

class QPushButton;

namespace core {

class WindowManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setAppEnvironment(state::StateMachine * stateMachine, wallet::Wallet * wallet);

    QWidget * getMainWindow();

    // Update tb & menu actions
    void updateActionStates(state::STATE actionState);

    void updateLeftBar(bool show);

private slots:
    void onNewNotificationMessage(notify::MESSAGE_LEVEL level, QString message);
    void onConfigUpdate();

    void on_listenerStatusButton_clicked();
    void on_nodeStatusButton_clicked();
    void on_helpButton_clicked();

    void updateListenerStatus(bool online);
    void updateNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

    void onUpdateSyncProgress(double progressPercent);

private:
    void updateListenerBtn();
    void updateNetworkName();

    enum class STATUS { IGNORE, RED, YELLOW, GREEN };
    void setStatusButtonState( QPushButton * btn, STATUS status, QString text );

private:
    Ui::MainWindow *ui;
    state::StateMachine * stateMachine = nullptr;
    wallet::Wallet * wallet = nullptr;

    bool leftBarShown = true;
};

}

#endif // MAINWINDOW_H

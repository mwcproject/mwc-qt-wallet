#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "../state/state.h"
#include "../wallet/wallet.h"

namespace Ui {
class MainWindow;
}

namespace state {
    class StateMachine;
}

namespace core {

class WindowManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setStateMachine(state::StateMachine * stateMachine);
    void setWallet(wallet::Wallet * wallet);

    QWidget * getMainWindow();

    // Update tb & menu actions
    void updateActionStates(state::STATE actionState);

    void updateLeftBar(bool show);

private slots:
    void on_actionVersion_triggered();

    void on_actionAccount_triggered();
    void on_actionEvents_triggered();
    void on_actionHODL_triggered();
    void on_actionSend_coins_triggered();
    void on_actionNode_status_triggered();
    void on_actionSend_recieve_offline_triggered();
    void on_actionListening_Status_triggered();
    void on_actionTransactions_triggered();
    void on_actionOutputs_triggered();
    void on_actionContacts_triggered();
    void on_actionConfig_triggered();
    void on_actionAirdrop_triggered();
    void on_actionRecieve_coins_triggered();

private slots:
    void onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message);


private:
    Ui::MainWindow *ui;
    state::StateMachine * stateMachine = nullptr;

    bool leftBarShown = true;
};

}

#endif // MAINWINDOW_H

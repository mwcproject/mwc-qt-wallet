#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "../state/state.h"
#include "../wallet/wallet.h"
#include "appcontext.h"

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
    //void on_actionVersion_triggered();

private slots:
    void onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message);


private:
    Ui::MainWindow *ui;
    state::StateMachine * stateMachine = nullptr;

    bool leftBarShown = true;
};

}

#endif // MAINWINDOW_H

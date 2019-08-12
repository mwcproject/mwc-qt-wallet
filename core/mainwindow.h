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


#ifdef Q_OS_MACOS
namespace Cocoa
{
    void changeTitleBarColor(WId winId, double red, double green, double blue);
}
#endif

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
    void onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message);

    void on_connectionStatusButton_clicked();
    void on_helpButton_clicked();

    void updateListenerStatus(bool online);

private:
    void updateListenerBtn();

private:
    Ui::MainWindow *ui;
    state::StateMachine * stateMachine = nullptr;
    wallet::Wallet * wallet = nullptr;

    bool leftBarShown = true;
};

}

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

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

    QWidget * getMainWindow();

    // Update tb & menu actions
    void updateActionStates();

private slots:
    void on_actionVersion_triggered();
    void updateClock();

    void on_actionAccount_triggered();
    void on_actionEvents_triggered();
    void on_actionHODL_triggered();
    void on_actionSend_coins_triggered();
    void on_actionNode_status_triggered();
    void on_actionSend_recieve_offline_triggered();
    void on_actionListening_Status_triggered();

    void on_actionTransactions_triggered();

private:
private:
    Ui::MainWindow *ui;
    QTimer *clockTimer = nullptr;
    state::StateMachine * stateMachine = nullptr;

    QAction * actionAccounts;
    QAction * actionEvents;
    QAction * actionHodl;
};

}

#endif // MAINWINDOW_H

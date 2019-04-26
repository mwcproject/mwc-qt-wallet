#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "walletwindow.h"

namespace Ui {
class MainWindow;
}
class WindowManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QWidget * getMainWindow();

private slots:
    void on_actionVersion_triggered();
    void updateClock();
    void on_nextBtn_clicked();

    void on_cancelBtn_clicked();

signals:
    void processNextStep( WalletWindowAction action );

private:
    Ui::MainWindow *ui;
    QTimer *clockTimer;
};

#endif // MAINWINDOW_H

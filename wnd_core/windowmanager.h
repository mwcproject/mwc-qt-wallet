#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include "../data/walletdata.h"
#include <QObject>
#include "walletwindow.h"
#include "mainwindow.h"
#include <QLayout>

class Wallet;

// WIndows menager is responsible for connection between UI and the data
class WindowManager : public QObject
{
    Q_OBJECT
public:
    WindowManager(WalletData & walData, Wallet * wallet, QWidget  * mainWnd );

    void start();

public slots:
    void processNextStep( WalletWindowAction action );

private:
    void switchToWindow( WalletWindow * newWindow );

private:
    WalletData & walletData;
    QWidget * mainWindow;
    Wallet * mwcWallet;

    WalletWindow * currentWnd = nullptr;
};

#endif // WINDOWMANAGER_H

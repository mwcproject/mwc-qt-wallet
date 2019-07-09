#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include "mainwindow.h"
#include <QLayout>

namespace wallet {
    class Wallet;
};

namespace core {

// WIndows menager is responsible for connection between UI and the data
class WindowManager : public QObject
{
    Q_OBJECT
public:
    WindowManager(QWidget  * mainWnd );

    // Show new window and return it
    QWidget * switchToWindowEx( QWidget * newWindow );
    // Parent for windows it can show.
    QWidget * getInWndParent() const;

    //void start();

    //void processNextStep( WalletWindowAction action );

private:

private:
    QWidget * mainWindow;
    QWidget * currentWnd = nullptr;
};

}

#endif // WINDOWMANAGER_H

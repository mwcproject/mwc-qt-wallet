#include "windowmanager.h"
#include "../wallet/wallet.h"
#include <QDebug>
#include <QApplication>
#include "../windows/connect2server.h"
#include "../windows/nodemanually_w.h"
#include "../windows/nodestatus_w.h"
#include "../windows/c_newwallet_w.h"
#include "../windows/c_newseed_w.h"
#include "../windows/c_enterseed.h"

namespace core {

WindowManager::WindowManager(QWidget  * mainWnd ) :
    mainWindow(mainWnd)
{
    Q_ASSERT(mainWindow);
}

QWidget * WindowManager::getInWndParent() const {
    return mainWindow;
}


QWidget * WindowManager::switchToWindowEx( QWidget * newWindow ) {
    if (currentWnd==newWindow)
        return newWindow;

    if (currentWnd!=nullptr) {
        currentWnd->close();
        currentWnd = nullptr;
    }
    if (newWindow==nullptr)
        return newWindow;

    currentWnd = newWindow;
    currentWnd->setAttribute( Qt::WA_DeleteOnClose );
    mainWindow->layout()->addWidget(currentWnd);
    currentWnd->show();
    return newWindow;
}

}


#include "global.h"
#include <QApplication>
#include <QMainWindow>

namespace mwc {

static QApplication * mwcApp = nullptr;
static QMainWindow * mwcMainWnd = nullptr;

void setApplication(QApplication * app, QMainWindow * mainWindow) {
    mwcApp = app;
    mwcMainWnd = mainWindow;
}

void closeApplication() {
    Q_ASSERT(mwcApp);
    Q_ASSERT(mwcMainWnd);

    // Async call is the only way to close App nicely !!!!
    // Alternatively we can call app::quit slot
    QMetaObject::invokeMethod(mwcMainWnd, "close", Qt::QueuedConnection);
}

}

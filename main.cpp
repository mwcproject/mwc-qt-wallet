#include "wnd_core/mainwindow.h"
#include <QApplication>
#include "data/walletdata.h"
#include "wnd_core/windowmanager.h"
#include "mwc_wallet/mockwallet.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWnd;

    WalletData data;

    MockWallet wallet;

    WindowManager wndManager(data, &wallet, mainWnd.getMainWindow() );

    mainWnd.show();
    wndManager.start();

    QObject::connect( &mainWnd, SIGNAL(processNextStep(WalletWindowAction)),
             &wndManager, SLOT(processNextStep(WalletWindowAction)) );

    return app.exec();
}

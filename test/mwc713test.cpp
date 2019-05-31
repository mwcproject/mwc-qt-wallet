#include "mwc713test.h"
#include <QApplication>
#include "../wallet/mwc713.h"
#include <QDebug>

// TEST for mwc713 interface.
// to run this test, please comment that app main and uncomment this one.

using namespace wallet;

/*int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //MWC713 wallet("tmp/notexist");

    MWC713 wallet("/mw/grin/wallet713/target/debug/mwc713");

    QPair<Wallet::InitWalletStatus, QString> res = wallet.open("floonet", "pass");
    qDebug() << "Wallet open result: " << res.first << "/" << res.second;

    return app.exec();
}*/

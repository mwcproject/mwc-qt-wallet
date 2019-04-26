#ifndef CONNECT2SERVER_H
#define CONNECT2SERVER_H

#include <QWidget>
#include "../wnd_core/walletwindow.h"

namespace Ui {
class connect2server;
}
class NodeConnection;

class ConnectToServer : public WalletWindow
{
    Q_OBJECT

public:
    explicit ConnectToServer(WalletWindowType appType, QWidget *parent, NodeConnection * dt);
    virtual ~ConnectToServer() override;

protected:
    bool validateData() override;
private:
    Ui::connect2server *ui;
    NodeConnection * data;
};

#endif // CONNECT2SERVER_H

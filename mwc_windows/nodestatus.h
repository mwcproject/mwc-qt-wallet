#ifndef LOCALNODESTATUS_H
#define LOCALNODESTATUS_H

#include <QWidget>
#include "../wnd_core/walletwindow.h"

namespace Ui {
class LocalNodeStatus;
}
class NodeConnection;

class NodeStatus : public WalletWindow
{
    Q_OBJECT

public:
    explicit NodeStatus(WalletWindowType appType, QWidget *parent, NodeConnection * dt);
    virtual ~NodeStatus() override;

protected:
    bool validateData() override;

private:
    Ui::LocalNodeStatus *ui;
    NodeConnection * data;
};

#endif // LOCALNODESTATUS_H

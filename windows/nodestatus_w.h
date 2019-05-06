#ifndef LOCALNODESTATUSW_H
#define LOCALNODESTATUSW_H

#include <QWidget>
#include "../wallet/wallet.h"

namespace Ui {
class LocalNodeStatus;
}

namespace state {
    class NodeStatus;
}


namespace wnd {

class NodeStatus : public QWidget
{
    Q_OBJECT

public:
    explicit NodeStatus(QWidget *parent, state::NodeStatus * state);
    virtual ~NodeStatus() override;

protected:
    void updateNodeStatus();

private slots:
    void on_changeNodeButton_clicked();

    void on_refreshButton_clicked();

private:
    Ui::LocalNodeStatus *ui;
    state::NodeStatus * state;
};

}

#endif // LOCALNODESTATUSW_H

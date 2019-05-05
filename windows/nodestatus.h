#ifndef LOCALNODESTATUS_H
#define LOCALNODESTATUS_H

#include <QWidget>

namespace Ui {
class LocalNodeStatus;
}

namespace wnd {

class NodeStatus : public QWidget
{
    Q_OBJECT

public:
    explicit NodeStatus(QWidget *parent);
    virtual ~NodeStatus() override;

protected:
    bool validateData();

private:
    Ui::LocalNodeStatus *ui;
};

}

#endif // LOCALNODESTATUS_H

#ifndef NODEMANUALLYW_H
#define NODEMANUALLYW_H

#include <QWidget>

namespace Ui {
class NodeManually;
}

namespace state {
    class NodeManually;
}

namespace wnd {

struct NodeAddr {
    QString host;
    int port = 0;

    void setData(QString _host, int _port) {host=_host; port=_port;}
};

class NodeManually : public QWidget
{
    Q_OBJECT

public:
    explicit NodeManually(QWidget *parent, state::NodeManually * state);
    virtual ~NodeManually() override;

private slots:
    void on_testConnectionButton_clicked();

    void on_cancelButton_clicked();

    void on_submitButton_clicked();

private:
    QPair<bool,NodeAddr> getNodeData();

private:
    Ui::NodeManually *ui;
    state::NodeManually * state;
};

}

#endif // NODEMANUALLYW_H

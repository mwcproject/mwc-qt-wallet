#ifndef CONNECT2SERVER_H
#define CONNECT2SERVER_H

#include <QWidget>

namespace Ui {
class connect2server;
}

namespace state {
    class Connect2Node;
}

namespace wnd {

class ConnectToServer : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectToServer(QWidget *parent, state::Connect2Node * state);
    virtual ~ConnectToServer() override;

private slots:
    void on_submitButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::connect2server *ui;
    state::Connect2Node * state;
};

}

#endif // CONNECT2SERVER_H

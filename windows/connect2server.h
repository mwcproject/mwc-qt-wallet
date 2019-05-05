#ifndef CONNECT2SERVER_H
#define CONNECT2SERVER_H

#include <QWidget>

namespace Ui {
class connect2server;
}

namespace wnd {

class ConnectToServer : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectToServer(QWidget *parent);
    virtual ~ConnectToServer() override;

protected:
    bool validateData();
private:
    Ui::connect2server *ui;
};

}

#endif // CONNECT2SERVER_H

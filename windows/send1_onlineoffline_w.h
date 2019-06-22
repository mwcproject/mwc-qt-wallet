#ifndef SEND1_ONLINEOFFLINE_W_H
#define SEND1_ONLINEOFFLINE_W_H

#include <QWidget>

namespace Ui {
class SendOnlineOffline;
}

namespace state {
    class SendOnlineOffline;
}

namespace wnd {

class SendOnlineOffline : public QWidget
{
    Q_OBJECT

public:
    explicit SendOnlineOffline(QWidget *parent, state::SendOnlineOffline * state);
    ~SendOnlineOffline();

    void showProgress();
private slots:
    void on_sendOnlineButton_clicked();
    void on_sendOfflineButton_clicked();

private:
    Ui::SendOnlineOffline *ui;
    state::SendOnlineOffline * state;
};

}

#endif // SEND1_ONLINEOFFLINE_W_H

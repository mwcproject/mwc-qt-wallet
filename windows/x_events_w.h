#ifndef EVENTSW_H
#define EVENTSW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Events;
}

namespace state {
    class Events;
}

namespace wnd {

class Events : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Events(QWidget *parent, state::Events * state);
    ~Events();

    void updateShowMessages();

private slots:
    void on_notificationList_cellActivated(int row, int column);

private:
    void initTableHeaders();
    void saveTableHeaders();

private:
    Ui::Events *ui;
    state::Events * state;

    QVector<wallet::WalletNotificationMessages> messages; // messaged that currently on diplay
};

}

#endif // EVENTS_H

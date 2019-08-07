#ifndef SEND3_OFFLINESETTINGS_H
#define SEND3_OFFLINESETTINGS_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class SendOffline;
}

namespace state {
class Send;
}


namespace wnd {

class SendOffline : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendOffline(QWidget *parent, const wallet::AccountInfo & selectedAccount, int64_t amount, state::Send * state);
    virtual ~SendOffline() override;

    void showSendMwcOfflineResult( bool success, QString message );
private slots:
    void on_settingsButton_clicked();
    void on_sendButton_clicked();

private:
    Ui::SendOffline * ui;
    state::Send * state;

    wallet::AccountInfo selectedAccount;
    int64_t amount;
};

}

#endif // SEND3_OFFLINESETTINGS_H

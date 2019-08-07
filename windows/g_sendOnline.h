#ifndef SEND2_ONLINE_H
#define SEND2_ONLINE_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class SendOnline;
}

namespace state {
class Send;
class Contacts;
}

namespace core {
struct SendCoinsParams;
}

namespace wnd {

class SendOnline : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendOnline(QWidget *parent, const wallet::AccountInfo & selectedAccount, int64_t amount, state::Send * state, state::Contacts * contactsState );
    virtual ~SendOnline() override;

    void sendRespond( bool success, const QStringList & errors );

private slots:
    void on_contactsButton_clicked();
    void on_settingsButton_clicked();
    void on_sendButton_clicked();

    void on_sendEdit_textEdited(const QString &arg1);
private:
    Ui::SendOnline *ui;
    state::Send * state = nullptr;
    state::Contacts * contactsState = nullptr;

    wallet::AccountInfo selectedAccount;
    int64_t amount;
};


}


#endif // SEND2_ONLINE_H

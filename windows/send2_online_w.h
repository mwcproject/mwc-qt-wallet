#ifndef SEND2_ONLINE_H
#define SEND2_ONLINE_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class SendOnline;
}

namespace state {
class SendOnline;
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
    explicit SendOnline(QWidget *parent, state::SendOnline * state, state::Contacts * contactsState );
    virtual ~SendOnline() override;

    void sendRespond( bool success, const QStringList & errors );

private slots:
    void on_contactsButton_clicked();
    void on_allAmountButton_clicked();
    void on_sendButton_clicked();

    void on_settingsButton2_clicked();

    void on_sendEdit_textEdited(const QString &arg1);

private:
    Ui::SendOnline *ui;
    state::SendOnline * state = nullptr;
    state::Contacts * contactsState = nullptr;
    QVector<wallet::AccountInfo> accountInfo;
};

QString generateAmountErrorMsg( int64_t mwcAmount, const wallet::AccountInfo & acc, const core::SendCoinsParams & sendParams );


}


#endif // SEND2_ONLINE_H

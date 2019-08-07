#ifndef SEND1_ONLINEOFFLINE_W_H
#define SEND1_ONLINEOFFLINE_W_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace Ui {
class SendStarting;
}

namespace state {
    class Send;
}

namespace wnd {

class SendStarting : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendStarting(QWidget *parent, state::Send * state);
    ~SendStarting();

    void updateAccountBalance( QVector<wallet::AccountInfo> accountInfo, const QString & selectedAccount );

private slots:

    void onChecked(int id);

    void on_nextButton_clicked();
    void on_allAmountButton_clicked();

private:
    Ui::SendStarting *ui;
    state::Send * state;
    QVector<wallet::AccountInfo> accountInfo;
};

QString generateAmountErrorMsg( int64_t mwcAmount, const wallet::AccountInfo & acc, const core::SendCoinsParams & sendParams );


}

#endif // SEND1_ONLINEOFFLINE_W_H

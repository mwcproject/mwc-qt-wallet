#ifndef K_ACCOUNTTRANSFER_W_H
#define K_ACCOUNTTRANSFER_W_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class AccountTransfer;
}

namespace state {
class AccountTransfer;
}

namespace wnd {

class AccountTransfer : public core::NavWnd
{
    Q_OBJECT

public:
    explicit AccountTransfer(QWidget *parent, state::AccountTransfer * state);
    ~AccountTransfer();

    void showTransferResults(bool ok, QString errMsg);
protected:
    void updateAccounts();

private slots:
    void on_allAmountButton_clicked();
    void on_settingsBtn_clicked();
    void on_transferButton_clicked();
    void on_backButton_clicked();

private:
    Ui::AccountTransfer *ui;
    state::AccountTransfer * state;
    QVector<wallet::AccountInfo> accountInfo;
};

}

#endif // K_ACCOUNTTRANSFER_W_H

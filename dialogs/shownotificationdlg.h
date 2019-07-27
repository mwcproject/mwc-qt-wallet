#ifndef SHOWNOTIFICATIONDLG_H
#define SHOWNOTIFICATIONDLG_H

#include <QDialog>
#include "../wallet/wallet.h"
#include "../control/mwcdialog.h"

namespace Ui {
class ShowNotificationDlg;
}

namespace dlg {

class ShowNotificationDlg : public control::MwcDialog {
Q_OBJECT

public:
    explicit ShowNotificationDlg(wallet::WalletNotificationMessages msg, QWidget *parent);

    ~ShowNotificationDlg();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ShowNotificationDlg *ui;
};

}

#endif // SHOWNOTIFICATIONDLG_H

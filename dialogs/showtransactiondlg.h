#ifndef SHOWTRANSACTIONDLG_H
#define SHOWTRANSACTIONDLG_H

#include "../control/mwcdialog.h"

namespace Ui {
class ShowTransactionDlg;
}

namespace wallet {
    struct WalletTransaction;
}

namespace dlg {

class ShowTransactionDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit ShowTransactionDlg(QWidget *parent,  const wallet::WalletTransaction & transaction);
    ~ShowTransactionDlg();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ShowTransactionDlg *ui;
};

}

#endif // SHOWTRANSACTIONDLG_H

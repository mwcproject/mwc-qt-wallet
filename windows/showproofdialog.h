#ifndef SHOWPROOFDIALOG_H
#define SHOWPROOFDIALOG_H

#include <QDialog>
#include "../wallet/wallet.h"

namespace Ui {
class ShowProofDialog;
}

namespace wnd {

class ShowProofDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShowProofDialog(QWidget *parent, QString fileName, wallet::WalletProofInfo walletProofInfo);
    ~ShowProofDialog();

private slots:
    void on_okButton_clicked();

private:
    Ui::ShowProofDialog *ui;
};

}

#endif // SHOWPROOFDIALOG_H

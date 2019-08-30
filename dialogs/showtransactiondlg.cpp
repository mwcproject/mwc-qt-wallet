#include "dialogs/showtransactiondlg.h"
#include "ui_showtransactiondlg.h"
#include "../wallet/wallet.h"
#include "../util/stringutils.h"

namespace dlg {

ShowTransactionDlg::ShowTransactionDlg(QWidget *parent,  const wallet::WalletTransaction & transaction) :
    control::MwcDialog(parent),
    ui(new Ui::ShowTransactionDlg)
{
    ui->setupUi(this);

    ui->titleLabel->setText("Transaction #" + QString::number(transaction.txIdx) );
    ui->type->setText( transaction.getTypeAsStr() );
    ui->id->setText( transaction.txid );
    ui->address->setText(transaction.address);
    ui->time->setText(transaction.confirmationTime);
    ui->mwc->setText( util::nano2one( transaction.coinNano ) );
    ui->confirm->setText( transaction.confirmed ? "Yes" : "No" );
}

ShowTransactionDlg::~ShowTransactionDlg()
{
    delete ui;
}

void ShowTransactionDlg::on_pushButton_clicked()
{
    accept();
}

}

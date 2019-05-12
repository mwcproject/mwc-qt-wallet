#include "windows/showproofdialog.h"
#include "ui_showproofdialog.h"
#include "../util/stringutils.h"

namespace wnd {

ShowProofDialog::ShowProofDialog(QWidget *parent, QString fileName, wallet::WalletProofInfo walletProofInfo) :
    QDialog(parent),
    ui(new Ui::ShowProofDialog)
{
    ui->setupUi(this);

    ui->fileNameEdit->setText(fileName);
    ui->coinsEdit->setText( util::nano2one( walletProofInfo.coinsNano ) );
    ui->fromEdit->setText( walletProofInfo.fromAddress);
    ui->toEdit->setText(walletProofInfo.toAddress);
    ui->outputEdit->setText(walletProofInfo.output);
    ui->kernelEdit->setText(walletProofInfo.kernel);

    setModal(true);
}

ShowProofDialog::~ShowProofDialog()
{
    delete ui;
}

void ShowProofDialog::on_okButton_clicked()
{
    accept();
}

}


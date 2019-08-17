#include "dialogs/fileslateinfodlg.h"
#include "ui_fileslateinfodlg.h"
#include "../util/stringutils.h"
#include "../util/Json.h"

namespace dlg {

FileSlateInfoDlg::FileSlateInfoDlg(QWidget *parent, const QString & title, const util::FileTransactionInfo & flTrInfo) :
    control::MwcDialog(parent),
    ui(new Ui::FileSlateInfoDlg)
{
    ui->setupUi(this);

    ui->titleLabel->setText(title);

    ui->amountText->setText( util::nano2one(flTrInfo.amount) );
    ui->txIdText->setText( flTrInfo.transactionId.left(flTrInfo.transactionId.indexOf('-'))  );
    ui->fileLocationText->setText( flTrInfo.fileName );
    ui->messageText->setText( flTrInfo.message );
}

FileSlateInfoDlg::~FileSlateInfoDlg()
{
    delete ui;
}

void FileSlateInfoDlg::on_cancelBtn_clicked()
{
    reject();
}

void FileSlateInfoDlg::on_processBtn_clicked()
{
    accept();
}

}


#include "windows/sendcoinsparamsdialog.h"
#include "ui_sendcoinsparamsdialog.h"
#include "../control/messagebox.h"

namespace wnd {

SendCoinsParamsDialog::SendCoinsParamsDialog(QWidget *parent, const core::SendCoinsParams & _params) :
    QDialog(parent),
    ui(new Ui::SendCoinsParamsDialog),
    params(_params)
{
    ui->setupUi(this);

    ui->inputsConfEdit->setText( QString::number(params.inputConfirmationNumber ) );
    ui->changeOutputsEdit->setText( QString::number( params.changeOutputs ) );
}

SendCoinsParamsDialog::~SendCoinsParamsDialog()
{
    delete ui;
}

void SendCoinsParamsDialog::on_okButton_clicked()
{
    bool ok = false;
    int confNumber = ui->inputsConfEdit->text().trimmed().toInt(&ok);
    if (!ok || confNumber<=0) {
        control::MessageBox::message(this, "Need info", "Please input correct value for minimum number of confirmations");
        return;
    }

    ok = false;
    int changeOutputs = ui->changeOutputsEdit->text().trimmed().toInt(&ok);
    if (!ok || changeOutputs<=0) {
        control::MessageBox::message(this, "Need info", "Please input correct value for change outputs");
        return;
    }

    params.setData( confNumber, changeOutputs );

    accept();
}

void SendCoinsParamsDialog::on_cancelButton_clicked()
{
    reject();
}

}


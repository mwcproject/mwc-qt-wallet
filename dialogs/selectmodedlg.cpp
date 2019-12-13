#include "dialogs/selectmodedlg.h"
#include "ui_selectmodedlg.h"

namespace dlg {

SelectModeDlg::SelectModeDlg(QWidget *parent) :
    control::MwcDialog(parent),
    ui(new Ui::SelectModeDlg)
{
    ui->setupUi(this);

    ui->radioOnlineWallet->setChecked(true);
}

SelectModeDlg::~SelectModeDlg()
{
    delete ui;
}

void SelectModeDlg::on_cancelButton_clicked()
{
    reject();
}

void SelectModeDlg::on_submitButton_clicked()
{
    if (ui->radioOnlineWallet->isChecked())
        runMode = config::WALLET_RUN_MODE::ONLINE_WALLET;
    else if (ui->radioOnlineNode->isChecked())
        runMode = config::WALLET_RUN_MODE::ONLINE_NODE;
    else if (ui->radioColdWallet->isChecked())
        runMode = config::WALLET_RUN_MODE::COLD_WALLET;
    else {
        Q_ASSERT(false);
    }

    accept();
}

}

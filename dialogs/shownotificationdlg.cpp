#include "dialogs/shownotificationdlg.h"
#include "ui_shownotificationdlg.h"

namespace dlg {

ShowNotificationDlg::ShowNotificationDlg(wallet::WalletNotificationMessages msg, QWidget *parent) :
        MwcDialog(parent),
        ui(new Ui::ShowNotificationDlg) {
    ui->setupUi(this);

    ui->timeText->setText( msg.time.toString("ddd MMMM d yyyy HH:mm:ss") );
    ui->levelText->setText( msg.getLevelLongStr() );
    ui->messageText->setPlainText( msg.message );
}

ShowNotificationDlg::~ShowNotificationDlg() {
    delete ui;
}

void ShowNotificationDlg::on_pushButton_clicked()
{
    accept();
}

}


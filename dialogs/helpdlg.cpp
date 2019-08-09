#include "dialogs/helpdlg.h"
#include "ui_helpdlg.h"

namespace dlg {

// documentName - doc name to display:  XXXXXX.html. Expected to be found at resources qrc:/help/XXXXXX.html
HelpDlg::HelpDlg( QWidget *parent, QString documentName ) :
    control::MwcDialog(parent),
    ui(new Ui::HelpDlg)
{
    ui->setupUi(this);
    ui->browser->setSource( QUrl("qrc:/help/" + documentName) );

    connect( ui->browser, &QTextBrowser::backwardAvailable, this, &HelpDlg::on_backwardAvailable );
    connect( ui->browser, &QTextBrowser::forwardAvailable, this, &HelpDlg::on_forwardAvailable );

    ui->forwardBtn->setEnabled(false);
    ui->backwardBtn->setEnabled(false);
}

HelpDlg::~HelpDlg()
{
    delete ui;
}

void HelpDlg::on_doneButton_clicked()
{
    accept();
}

void HelpDlg::on_backwardBtn_clicked()
{
    ui->browser->backward();
}

void HelpDlg::on_forwardBtn_clicked()
{
    ui->browser->forward();
}

void HelpDlg::on_backwardAvailable(bool available) {
    ui->backwardBtn->setEnabled(available);
}
void HelpDlg::on_forwardAvailable(bool available) {
    ui->forwardBtn->setEnabled(available);
}

}


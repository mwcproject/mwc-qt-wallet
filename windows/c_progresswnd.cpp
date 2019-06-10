#include "windows/c_progresswnd.h"
#include "ui_c_progresswnd.h"

namespace wnd {

ProgressWnd::ProgressWnd(QWidget *parent, QString header, QString msgProgress, QString msgPlus, IProgressWndState * callback ) :
    QWidget(parent),
    ui(new Ui::ProgressWnd),
    cancelCallback(callback)
{
    ui->setupUi(this);

    setHeader(header);
    updateProgress(0,msgProgress);

    setMsgPlus(msgPlus);

    if (cancelCallback==nullptr)
        ui->cancelButton->hide();
}

ProgressWnd::~ProgressWnd()
{
    delete ui;
}

void ProgressWnd::on_cancelButton_clicked()
{
    if (cancelCallback)
        cancelCallback->cancelProgress();
}

void ProgressWnd::setHeader(QString header) {
    if (header.length()>0) {
        ui->header->setText(header);
        ui->header->show();
    }
    else {
        ui->header->hide();
    }

}
void ProgressWnd::setMsgPlus(QString msgPlus) {
    if (msgPlus.length()>0) {
        ui->messagePlus->setText(msgPlus);
        ui->messagePlus->show();
    }
    else {
        ui->messagePlus->hide();
    }
}

void ProgressWnd::initProgress(int min, int max) {
    ui->progressBar->setRange(min,max);
}

void ProgressWnd::updateProgress(int pos, QString msgProgress) {
    ui->progressBar->setValue(pos);

    if (msgProgress.length()>0) {
        ui->messageProgress->setText(msgProgress);
        ui->messageProgress->show();
    }
    else {
        ui->messageProgress->hide();
    }
}



}

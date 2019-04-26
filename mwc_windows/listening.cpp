#include "mwc_windows/listening.h"
#include "ui_listening.h"

Listening::Listening(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Listening)
{
    ui->setupUi(this);
}

Listening::~Listening()
{
    delete ui;
}

void Listening::on_nextAddressButton_clicked()
{

}

void Listening::on_mwcAddressFromIndexButton_clicked()
{

}

#include "listening.h"
#include "ui_listening.h"

namespace wnd {

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

}

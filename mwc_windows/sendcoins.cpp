#include "mwc_windows/sendcoins.h"
#include "ui_sendcoins.h"

SendCoins::SendCoins(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendCoins)
{
    ui->setupUi(this);
}

SendCoins::~SendCoins()
{
    delete ui;
}

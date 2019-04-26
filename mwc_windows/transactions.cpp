#include "mwc_windows/transactions.h"
#include "ui_transactions.h"

Transactions::Transactions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Transactions)
{
    ui->setupUi(this);
}

Transactions::~Transactions()
{
    delete ui;
}

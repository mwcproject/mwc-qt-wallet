#include "mwc_windows/accounts.h"
#include "ui_accounts.h"

Accounts::Accounts(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Accounts)
{
    ui->setupUi(this);
}

Accounts::~Accounts()
{
    delete ui;
}

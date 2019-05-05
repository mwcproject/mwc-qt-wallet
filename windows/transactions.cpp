#include "transactions.h"
#include "ui_transactions.h"

namespace wnd {

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

}

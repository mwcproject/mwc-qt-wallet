#include "core/navmenyaccount.h"
#include "ui_navmenyaccount.h"

NavMenyAccount::NavMenyAccount(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NavMenyAccount)
{
    ui->setupUi(this);
}

NavMenyAccount::~NavMenyAccount()
{
    delete ui;
}

#include "mwc_windows/hodl.h"
#include "ui_hodl.h"

Hodl::Hodl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Hodl)
{
    ui->setupUi(this);
}

Hodl::~Hodl()
{
    delete ui;
}

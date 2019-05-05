#include "claiming.h"
#include "ui_claiming.h"

namespace wnd {

Claiming::Claiming(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Claiming)
{
    ui->setupUi(this);
}

Claiming::~Claiming()
{
    delete ui;
}

}

#include "outputs.h"
#include "ui_outputs.h"

namespace wnd {

Outputs::Outputs(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Outputs)
{
    ui->setupUi(this);
}

Outputs::~Outputs()
{
    delete ui;
}

}

#include "mwc_windows/outputs.h"
#include "ui_outputs.h"

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

#include "mwc_windows/events.h"
#include "ui_events.h"

Events::Events(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Events)
{
    ui->setupUi(this);
}

Events::~Events()
{
    delete ui;
}

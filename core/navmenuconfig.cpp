#include "core/navmenuconfig.h"
#include "ui_navmenuconfig.h"

NavMenuConfig::NavMenuConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NavMenuConfig)
{
    ui->setupUi(this);
}

NavMenuConfig::~NavMenuConfig()
{
    delete ui;
}

#include "nodestatus.h"
#include "ui_nodestatus.h"

NodeStatus::NodeStatus(WalletWindowType appType, QWidget *parent, NodeConnection * dt) :
    WalletWindow (appType, parent),
    ui(new Ui::LocalNodeStatus),
    data(dt)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
}

NodeStatus::~NodeStatus()
{
    delete ui;
}

bool NodeStatus::validateData() {
    return true;
}

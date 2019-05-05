#include "nodestatus.h"
#include "ui_nodestatus.h"

namespace wnd {

NodeStatus::NodeStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocalNodeStatus)
{
    ui->setupUi(this);
}

NodeStatus::~NodeStatus()
{
    delete ui;
}

bool NodeStatus::validateData() {
    return true;
}

}



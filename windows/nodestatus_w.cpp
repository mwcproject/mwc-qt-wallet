#include "nodestatus_w.h"
#include "ui_nodestatus.h"
#include "../state/nodestatus.h"

namespace wnd {

NodeStatus::NodeStatus(QWidget *parent, state::NodeStatus * _state) :
    QWidget(parent),
    ui(new Ui::LocalNodeStatus),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Node status");

//    ui->serverLabel->setText( state->getWalletConfig().mwcNodeURI );
    updateNodeStatus();
}

NodeStatus::~NodeStatus()
{
    delete ui;
}

void NodeStatus::on_changeNodeButton_clicked()
{
    state->requestToChangeTheNode();
}

void NodeStatus::on_refreshButton_clicked()
{
    updateNodeStatus();
}

void NodeStatus::updateNodeStatus() {
    wallet::NodeStatus status = state->getNodeStatus();

    ui->blocksLabel->setText( QString::number(status.height) + " blocks" );
    ui->statusLabel->setText( status.tip );
    ui->connectionsLabel->setText( QString::number(status.connection) );
    ui->difficultyLabel->setText( QString::number( status.total_difficulty ) );
}



}



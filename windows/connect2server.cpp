#include "connect2server.h"
#include "ui_connect2server.h"
#include "../control/messagebox.h"
#include  "../state/connect2node.h"

namespace wnd {

ConnectToServer::ConnectToServer(QWidget *parent, state::Connect2Node * _state) :
    QWidget(parent),
    ui(new Ui::connect2server),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Connect to MWC node");

    ui->cancelButton->setFocus();
}

ConnectToServer::~ConnectToServer()
{
    delete ui;
}

void ConnectToServer::on_submitButton_clicked()
{
    if (ui->radioSelectManually->isChecked()) {
        state->applyChoice( state::Connect2Node::NODE_MANUAL);
    }
    else if (ui->radioStandalone->isChecked()) {
        state->applyChoice( state::Connect2Node::NODE_LOCAL);
    }
    else if (ui->radioAutoConnect->isChecked()) {
        state->applyChoice( state::Connect2Node::NODE_POOL);
    }
    else {
        control::MessageBox::message(this, "Error",
                              "Please select the connection type to your server");
    }

}

void ConnectToServer::on_cancelButton_clicked()
{
    state->cancel();
}


}


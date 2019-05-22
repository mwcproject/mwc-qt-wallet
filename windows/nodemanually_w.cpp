#include "windows/nodemanually_w.h"
#include "ui_nodemanually.h"
#include <QMessageBox>
#include "../state/nodemanually.h"

namespace wnd {

NodeManually::NodeManually(QWidget *parent, state::NodeManually * _state) :
    QWidget(parent),
    ui(new Ui::NodeManually),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("MWC node to connect");

    ui->hostEdit->setFocus();
}

NodeManually::~NodeManually()
{
    delete ui;
}

QPair<bool,NodeAddr> NodeManually::getNodeData() {
    QString host = ui->hostEdit->text();
    QString portStr = ui->portEdit->text();

    if (host.length() == 0 || portStr.length()==0) {
        QMessageBox::information(this, "Connection to MWC node",
                                 "Please specify the host and port for the node to connect");
        return QPair<bool,NodeAddr>(false, NodeAddr());
    }

    bool ok = false;
    int port = portStr.toInt(&ok);

    Q_UNUSED(port);

    if (!ok) {
        QMessageBox::information(this, "Connection to MWC node",
                                 "Please specify the port number as a positive integer");
        return QPair<bool,NodeAddr>(false, NodeAddr());
    }

    NodeAddr na;
    na.setData(host,port);
    return QPair<bool,NodeAddr>(true, na);
}

void NodeManually::on_testConnectionButton_clicked()
{
    QPair<bool,NodeAddr> data = getNodeData();
    if (data.first) {
        QPair<bool,QString> testRes = state->testMwcNode(data.second.host, data.second.port);
        if (testRes.first) {
            QMessageBox::information(this, "MWC node connection", "Your MWC node is reachable");
            return;
        }
        else {
            QMessageBox::critical(this, "MWC node connection", "This MWC node is not reachable. Error: " + testRes.second);
            return;
        }
    }
}

void NodeManually::on_cancelButton_clicked()
{
    state->cancel();
}

void NodeManually::on_submitButton_clicked()
{
    QPair<bool,NodeAddr> data = getNodeData();
    if (data.first) {
        state->submitMwcNode(data.second.host, data.second.port);
    }
}


}


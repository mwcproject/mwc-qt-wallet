#include "windows/nodemanually.h"
#include "ui_nodemanually.h"
#include <QMessageBox>

namespace wnd {

NodeManually::NodeManually(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NodeManually)
{
    ui->setupUi(this);
}

NodeManually::~NodeManually()
{
    delete ui;
}

bool NodeManually::validateData() {
    QString host = ui->hostEdit->text();
    QString portStr = ui->portEdit->text();

    if (host.length() == 0 || portStr.length()==0) {
        QMessageBox::information(this, "Connection to MWC node",
                                 "Please specify the host and port for the node to connect");
        return false;
    }

    bool ok = false;
    int port = portStr.toInt(&ok);

    Q_UNUSED(port);

    if (!ok) {
        QMessageBox::information(this, "Connection to MWC node",
                                 "Please specify the port number as a positive integer");
        return false;
    }

    return ok;
}

void NodeManually::on_testConnectionButton_clicked()
{
    if (!validateData())
        return;

   // QMessageBox::information(this, "Connection to MWC node", "Connection to the node " +
     //                        data->getHost() + ":" + QString::number(data->getPort()) + " was succeeded." );
}

}

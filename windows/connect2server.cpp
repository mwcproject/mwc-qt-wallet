#include "connect2server.h"
#include "ui_connect2server.h"
#include <QMessageBox>

namespace wnd {

ConnectToServer::ConnectToServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::connect2server)
{
    ui->setupUi(this);
}

ConnectToServer::~ConnectToServer()
{
    delete ui;
}

bool ConnectToServer::validateData() {
    if (ui->radioSelectManually->isChecked()) {
//        data->setNode( NodeConnection::SELECTED );
    }
    else if (ui->radioStandalone->isChecked()) {
  //      data->setNode( NodeConnection::LOCAL );
    }
    else if (ui->radioAutoConnect->isChecked()) {
    //    data->setNode( NodeConnection::WMC_POOL );
    }
    else {
        QMessageBox::critical(this, "Connecto to server",
                              "Please select the connection type to your server");
        return false;
    }

    return true;
}

}

#include "walletconfig_w.h"
#include "ui_walletconfig.h"
#include "../state/walletconfig.h"
#include <QMessageBox>

namespace wnd {

WalletConfig::WalletConfig(QWidget *parent, state::WalletConfig * _state) :
    QWidget(parent),
    ui(new Ui::WalletConfig),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Wallet config");

    wallet::WalletConfig cfg = state->getWalletConfig();

    ui->walletFolder->setText( cfg.dataPath );
    ui->mwcboxHost->setText( cfg.mwcboxDomain );
    ui->mwcboxPort->setText( QString::number(cfg.mwcboxPort) );
}

WalletConfig::~WalletConfig()
{
    delete ui;
}

void wnd::WalletConfig::on_updateButton_clicked()
{
    QString folder = ui->walletFolder->text();
    QString boxDomain = ui->mwcboxHost->text();
    QString boxPortStr = ui->mwcboxPort->text();

    bool ok = false;
    int boxPort = boxPortStr.toInt(&ok);

    if ( folder.length()==0 ) {
        QMessageBox::critical(this, "Incorrect wallet config",
                              "Please specify non empty folder name for wallet data");
        return;
    }

    if ( boxDomain.length()==0 ) {
        QMessageBox::critical(this, "Incorrect wallet config",
                              "Please specify non empty MWC box domain name");
        return;
    }

    if (!( ok && boxPort>0 && boxPort<65535 )) {
        QMessageBox::critical(this, "Incorrect wallet config",
                              "Please specify valid port number for MWC box");
        return;
    }

    wallet::WalletConfig cfg = state->getWalletConfig();

    cfg.dataPath = folder;
    cfg.mwcboxDomain = boxDomain;
    cfg.mwcboxPort = boxPort;

    QPair<bool, QString> res = state->setWalletConfig(cfg);

    if (!res.first) {
        QMessageBox::critical(this, "Wallet Config", "Unable to update Wallet Config. Error: " + res.second);
        return;
    }
}

}


#include "walletconfig_w.h"
#include "ui_walletconfig.h"
#include "../state/walletconfig.h"
#include "../control/messagebox.h"

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
    QString folder = ui->walletFolder->text().trimmed();
    QString boxDomain = ui->mwcboxHost->text().trimmed();
    QString boxPortStr = ui->mwcboxPort->text().trimmed();

    bool ok = false;
    int boxPort = boxPortStr.toInt(&ok);

    if ( folder.length()==0 ) {
        control::MessageBox::message(this, "Need info",
                              "Please specify non empty folder name for wallet data");
        return;
    }

    if ( boxDomain.length()==0 ) {
        control::MessageBox::message(this, "Need info",
                              "Please specify non empty MWC box domain name");
        return;
    }

    if (!( ok && boxPort>0 && boxPort<65535 )) {
        control::MessageBox::message(this, "Need info",
                              "Please specify valid port number for MWC box");
        return;
    }

    wallet::WalletConfig cfg = state->getWalletConfig();

    cfg.dataPath = folder;
    cfg.mwcboxDomain = boxDomain;
    cfg.mwcboxPort = boxPort;

    QPair<bool, QString> res = state->setWalletConfig(cfg);

    if (!res.first) {
        control::MessageBox::message(this, "Error", "Unable to update Wallet Config.\nError: " + res.second);
        return;
    }
}

}


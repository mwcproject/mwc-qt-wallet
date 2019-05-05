#include "walletconfig.h"
#include "ui_walletconfig.h"

namespace wnd {

WalletConfig::WalletConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WalletConfig)
{
    ui->setupUi(this);
}

WalletConfig::~WalletConfig()
{
    delete ui;
}

}

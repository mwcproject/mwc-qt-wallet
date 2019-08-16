#include "x_walletinstances.h"
#include "ui_x_walletinstances.h"
#include "../state/x_walletconfig.h"
#include "../state/timeoutlock.h"
#include <QFileDialog>
#include "../control/messagebox.h"

namespace dlg {

WalletInstances::WalletInstances(QWidget *parent, state::WalletConfig * _state) :
        control::MwcDialog( parent ),
        ui( new Ui::WalletInstances ),
        state( _state )
{
    ui->setupUi(this);

    ui-> mwc713directoryEdit->setText( state->getWalletConfig().dataPath );
}

WalletInstances::~WalletInstances() {
    delete ui;
}

void WalletInstances::on_mwc713directorySelect_clicked() {
    state::TimeoutLockObject to( state );

    QString basePath = ioutils::getAppDataPath();
    QString dir = QFileDialog::getExistingDirectory(
            nullptr,
            "Select your wallet folder name",
            basePath);
    if (dir.isEmpty())
        return;

    QDir baseDir(basePath);

    ui->mwc713directoryEdit->setText( baseDir.relativeFilePath(dir) );
}

void WalletInstances::on_cancelButton_clicked() {
    reject();
}

void WalletInstances::on_applyButton_clicked() {

    QString dataPath = ui->mwc713directoryEdit->text();

    if (dataPath.isEmpty()) {
        control::MessageBox::message( this, "Input error", "Please specify folder name for your wallet" );
        ui-> mwc713directoryEdit->setFocus();
        return;
    }

    wallet::WalletConfig newWalletConfig = state->getWalletConfig();
    if ( dataPath == newWalletConfig.dataPath ) {
        reject();
        return; // no changes was made
    }

    newWalletConfig.dataPath = dataPath;

    if (!state->setWalletConfig(newWalletConfig) )
    {
        control::MessageBox::message( this, "Internal error", "Unable to change the folder for your wallet" );
        return;
    }
    accept();
}


}

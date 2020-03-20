// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "x_walletinstances.h"
#include "ui_x_walletinstances.h"
#include "../state/x_walletconfig.h"
#include "../state/timeoutlock.h"
#include <QFileDialog>
#include "../control/messagebox.h"
#include "../dialogs/networkselectiondlg.h"
#include "../util/Process.h"

namespace dlg {

WalletInstances::WalletInstances(QWidget *parent, state::WalletConfig * _state) :
        control::MwcDialog( parent ),
        ui( new Ui::WalletInstances ),
        state( _state )
{
    ui->setupUi(this);

    ui-> mwc713directoryEdit->setText( state->getWalletConfig().getDataPath() );
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
    QString walletDataDir = baseDir.relativeFilePath(dir);

    QString runningArc = util::getBuildArch();
    QString dataArc    = wallet::WalletConfig::readNetworkArchFromDataPath(walletDataDir).second;
    if ( runningArc != dataArc ) {
        control::MessageBox::messageText(nullptr, "Wallet data architecture mismatch",
                    "Your mwc713 seed at '"+ walletDataDir +"' was created with "+ dataArc+" bits version of the wallet. You are using " + runningArc + " bit version.");
        return;
    }

    ui->mwc713directoryEdit->setText( walletDataDir );
}

void WalletInstances::on_cancelButton_clicked() {
    reject();
}

void WalletInstances::on_applyButton_clicked() {

    QString dataPath = ui->mwc713directoryEdit->text();

    if (dataPath.isEmpty()) {
        control::MessageBox::messageText( this, "Input error", "Please specify folder name for your wallet" );
        ui-> mwc713directoryEdit->setFocus();
        return;
    }

    wallet::WalletConfig newWalletConfig = state->getWalletConfig();
    if ( dataPath == newWalletConfig.getDataPath() ) {
        reject();
        return; // no changes was made
    }

    // Data path need to be updated, as well as a network
    QPair<QString,QString> networkArch = wallet::WalletConfig::readNetworkArchFromDataPath(dataPath); // local path as writen in config
    QString runningArc = util::getBuildArch();

    // Just in case. Normally will never be called
    if ( runningArc != networkArch.second ) {
        control::MessageBox::messageText(nullptr, "Wallet data architecture mismatch",
                                     "Your mwc713 seed at '"+ dataPath +"' was created with "+ networkArch.second+" bits version of the wallet. You are using " + runningArc + " bit version.");
        return;
    }

    if (networkArch.first.isEmpty()) {
        // Check if seed file does exist. Import of the data?
        if ( wallet::WalletConfig::doesSeedExist(dataPath) ) {

            dlg::NetworkSelectionDlg nwDlg(this);
            if (nwDlg.exec() != QDialog::Accepted)
                return;

            networkArch.first = nwDlg.getNetwork() == state::InitAccount::MWC_NETWORK::MWC_MAIN_NET ? "Mainnet" : "Floonet";

        }
        else
            networkArch.first = "Mainnet"; // will be redefined later in any case...

        wallet::WalletConfig::saveNetwork2DataPath(dataPath, networkArch.first, util::getBuildArch() );
    }

    newWalletConfig.setDataPathWithNetwork( dataPath, networkArch.first );

    if (!state->setWalletConfig(newWalletConfig, false) )
    {
        control::MessageBox::messageText( this, "Internal error", "Unable to change the folder for your wallet" );
        return;
    }
    accept();
}


}

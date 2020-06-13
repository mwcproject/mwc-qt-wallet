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
#include "../util_desktop/timeoutlock.h"
#include <QFileDialog>
#include "../control_desktop/messagebox.h"
#include "../dialogs_desktop/networkselectiondlg.h"
#include "../bridge/wallet_b.h"
#include "../bridge/wnd/x_walletconfig_b.h"
#include "../bridge/config_b.h"

namespace dlg {

WalletInstances::WalletInstances(QWidget *parent) :
        control::MwcDialog( parent ),
        ui( new Ui::WalletInstances )
{
    ui->setupUi(this);
    wallet = new bridge::Wallet(this);
    //util = new bridge::Util(this);
    config = new bridge::Config(this);
    walletConfig = new bridge::WalletConfig(this);

    ui-> mwc713directoryEdit->setText( config->getDataPath() );
}

WalletInstances::~WalletInstances() {
    delete ui;
}

void WalletInstances::on_mwc713directorySelect_clicked() {
    util::TimeoutLockObject to( "WalletInstances");

    QString basePath = config->getAppDataPath();
    if (basePath.startsWith(' ')) {
        control::MessageBox::messageText(nullptr, "Error", basePath);
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(
            nullptr,
            "Select your wallet folder name",
            basePath);

    if (dir.isEmpty())
        return;

    auto dirOk = util::validateMwc713Str(dir);
    if (!dirOk.first) {
        core::getWndManager()->messageTextDlg("Directory Name",
                                              "This directory name is not acceptable.\n" + dirOk.second);
        return;
    }

    QDir baseDir(basePath);
    QString walletDataDir = baseDir.relativeFilePath(dir);

    QString runningArc = config->getBuildArch();
    QString dataArc    = config->readNetworkArchFromDataPath(walletDataDir)[1];
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

    auto res = util::validateMwc713Str(dataPath);
    if (!res.first) {
        control::MessageBox::messageText(this, "Wallet data path",
                                         "Wallet path is invalid.\n\n" + res.second);
        return;
    }

    if ( dataPath == config->getDataPath() ) {
        reject();
        return; // no changes was made
    }

    // Data path need to be updated, as well as a network
    // [<network>, <architecture>]
    QVector<QString> networkArch = config->readNetworkArchFromDataPath(dataPath); // local path as writen in config
    QString runningArc = config->getBuildArch();

    // Just in case. Normally will never be called
    if ( runningArc != networkArch[1] ) {
        control::MessageBox::messageText(this, "Wallet data architecture mismatch",
                                     "Your mwc713 seed at '"+ dataPath +"' was created with "+ networkArch[1]+" bits version of the wallet. You are using " + runningArc + " bit version.");
        return;
    }

    if (networkArch[0].isEmpty()) {
        // Check if seed file does exist. Import of the data?
        if ( config->doesSeedExist(dataPath) ) {
            dlg::NetworkSelectionDlg nwDlg(this);
            if (nwDlg.exec() != QDialog::Accepted)
                return;

            networkArch[0] = nwDlg.getNetwork() == state::InitAccount::MWC_NETWORK::MWC_MAIN_NET ? "Mainnet" : "Floonet";
        }
        else
            networkArch[0] = "Mainnet"; // will be redefined later in any case...

        config->saveNetwork2DataPath(dataPath, networkArch[0], config->getBuildArch() );
    }

    if (!walletConfig->setDataPathWithNetwork( dataPath, networkArch[0], false ) )
    {
        control::MessageBox::messageText( this, "Internal error", "Unable to change folder for your wallet" );
        return;
    }
    accept();
}


}

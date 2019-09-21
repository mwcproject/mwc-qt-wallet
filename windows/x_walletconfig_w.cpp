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

#include "x_walletconfig_w.h"
#include "ui_x_walletconfig.h"
#include "../state/x_walletconfig.h"
#include "../control/messagebox.h"
#include <QFileInfo>
#include <QHostInfo>
#include "../util/Waiting.h"
#include "../util/Process.h"
#include <QFileDialog>
#include <QStandardPaths>
#include "../state/timeoutlock.h"
#include "../dialogs/networkselectiondlg.h"
#include "../core/Config.h"

namespace wnd {

static int scale2Id(double scale) {
    if (scale < 1.15 )
        return 1;
    else if (scale < 1.45)
        return 2;
    else if (scale < 1.8)
        return 3;
    else
        return 4;
}

static double id2scale(int scale) {
    switch(scale) {
    case 1: return 1.0;
    case 2: return 1.3;
    case 3: return 1.6;
    default: return 2.0;
    }
}

static QString calcMWCMW_DOMAIN_DEFAULT_STR() {
    return QString("default MWC MQ") + (config::getUseMwcMqS() ? "S" : "")  + " Domain";
}

WalletConfig::WalletConfig(QWidget *parent, state::WalletConfig * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::WalletConfig),
    state(_state)
{
    ui->setupUi(this);

    currentWalletConfig = state->getWalletConfig();
    sendParams = state->getSendCoinsParams();

    defaultWalletConfig = state->getDefaultWalletConfig();

    uiScale = scale2Id( state->getGuiScale() );
    checkSizeButton(uiScale);

#ifdef Q_OS_DARWIN
    // MacOS doesn't support font scale. Need to hide all the buttons
    ui->fontSizeLabel->hide();
    ui->fontSz1->hide();
    ui->fontSz2->hide();
    ui->fontSz3->hide();
    ui->fontSz4->hide();
#endif

    setValues(currentWalletConfig.getDataPath(), currentWalletConfig.keyBasePath,
              currentWalletConfig.getMwcMqHostNorm(),
              sendParams.inputConfirmationNumber, sendParams.changeOutputs);
    updateButtons();
}

WalletConfig::~WalletConfig()
{
    delete ui;
}

void WalletConfig::setValues(const QString & mwc713directory,
                             const QString & keyBasePath,
                             const QString & mwcmqHostNorm,
                             int inputConfirmationNumber,
                             int changeOutputs) {
    ui->mwc713directoryEdit->setText( mwc713directory );
    ui->keybasePathEdit->setText( keybasePathConfig2InputStr(keyBasePath) );
    ui->mwcmqHost->setText( mwcDomainConfig2InputStr( mwcmqHostNorm ) );

    ui->confirmationNumberEdit->setText( QString::number(inputConfirmationNumber) );
    ui->changeOutputsEdit->setText( QString::number(changeOutputs) );

    setFocus();
}

void WalletConfig::updateButtons() {
    bool sameWithCurrent =
        getcheckedSizeButton() == uiScale &&
        ui->mwc713directoryEdit->text().trimmed() == currentWalletConfig.getDataPath() &&
        keybasePathInputStr2Config( ui->keybasePathEdit->text().trimmed() ) == currentWalletConfig.keyBasePath &&
        mwcDomainInputStr2Config( ui->mwcmqHost->text().trimmed() ) == currentWalletConfig.getMwcMqHostNorm() &&
        ui->confirmationNumberEdit->text().trimmed() == QString::number(sendParams.inputConfirmationNumber) &&
        ui->changeOutputsEdit->text().trimmed() == QString::number(sendParams.changeOutputs);

    bool sameWithDefault =
        getcheckedSizeButton() == scale2Id( state->getInitGuiScale() ) &&
        ui->mwc713directoryEdit->text().trimmed() == defaultWalletConfig.getDataPath() &&
        keybasePathInputStr2Config( ui->keybasePathEdit->text().trimmed() ) == defaultWalletConfig.keyBasePath &&
        mwcDomainInputStr2Config( ui->mwcmqHost->text().trimmed() ) == defaultWalletConfig.getMwcMqHostNorm() &&
        ui->confirmationNumberEdit->text().trimmed() == QString::number(defaultSendParams.inputConfirmationNumber) &&
        ui->changeOutputsEdit->text().trimmed() == QString::number(defaultSendParams.changeOutputs);

    ui->restoreDefault->setEnabled( !sameWithDefault );
    ui->applyButton->setEnabled( !sameWithCurrent );
}


QString WalletConfig::mwcDomainConfig2InputStr(QString mwcDomain) {
    return mwcDomain.isEmpty() ? calcMWCMW_DOMAIN_DEFAULT_STR() : mwcDomain;
}

QString WalletConfig::mwcDomainInputStr2Config(QString mwcDomain) {
    return mwcDomain == calcMWCMW_DOMAIN_DEFAULT_STR() ? "" : mwcDomain;
}

QString WalletConfig::keybasePathConfig2InputStr(QString kbpath) {
    if (kbpath.isEmpty())
        return "default";

    QFileInfo check(kbpath);
    if (check.exists() && check.isFile())
        return kbpath;

    return "Not Found";
}

QString WalletConfig::keybasePathInputStr2Config(QString kbpath) {
    if (kbpath == "default")
        return "";

    if (kbpath == "Not Found")
        return currentWalletConfig.keyBasePath;

    return kbpath;
}


// return true if data is fine. In case of error will show message for the user
bool WalletConfig::readInputValue( const wallet::WalletConfig & prevWalletConfig,
                    wallet::WalletConfig & newWalletConfig, core::SendCoinsParams & newSendParams ) {
    state::TimeoutLockObject to( state );

    // mwc713 directory
    QString walletDir = ui->mwc713directoryEdit->text().trimmed();
    if (walletDir.isEmpty()) {
        control::MessageBox::message(this, "Input", "Please specify non empty wallet folder name");
        ui->mwc713directoryEdit->setFocus();
        return false;
    }

    QPair <bool, QString> res = util::validateMwc713Str(walletDir);
    if (!res.first) {
        control::MessageBox::message( this, "Input", res.second );
        ui->mwc713directoryEdit->setFocus();
        return false;
    }

    // keybase path
    QString keybasePath = keybasePathInputStr2Config( ui->keybasePathEdit->text().trimmed() );
    res = util::validateMwc713Str(keybasePath);
    if (!res.first) {
        control::MessageBox::message( this, "Input", res.second );
        ui->keybasePathEdit->setFocus();
        return false;
    }

    #ifdef Q_OS_WIN

        // Under the windows we want user select Client, Not a GUI
        // C:\Users\XXXX\AppData\Local\Keybase\keybase.exe        - ok
        // C:\Users\XXXX\AppData\Local\Keybase\Gui\Keybase.exe    - GUI, will not work
        if (keybasePath.contains("Gui") || keybasePath.contains("gui") ) {
            if ( control::MessageBox::RETURN_CODE::BTN1 == control::MessageBox::question( this, "Keybase path, Warning",
                               "Wallet requires keybase console client. Seems like you selected keybase GUI that doesn't provide needed functionality. Please double check if console client path was selected.",
                               "Cancel", "Use this path", true, false ) )
                return false;
        }
    #endif


    QString mwcmqHost = mwcDomainInputStr2Config(ui->mwcmqHost->text().trimmed());
    if (!mwcmqHost.isEmpty()) {
        // Checking the host

        util::Waiting w; // Host verification might tale time, what is why waiting here

        QHostInfo host = QHostInfo::fromName(mwcmqHost);
        if (host.error() != QHostInfo::NoError) {
            control::MessageBox::message( this, "Input", "Host "+mwcmqHost+" is not reachable.\n" + host.errorString() );
            ui->mwcmqHost->setFocus();
            return false;
        }
    }

    bool ok = false;
    int confirmations = ui->confirmationNumberEdit->text().trimmed().toInt(&ok);
    if (!ok || confirmations<=0 || confirmations>10) {
        control::MessageBox::message( this, "Input", "Please input the number of confirmations in the range from 1 to 10" );
        ui->confirmationNumberEdit->setFocus();
        return false;
    }

    int changeOutputs = ui->changeOutputsEdit->text().trimmed().toInt(&ok);
    if (!ok || changeOutputs<=0 || confirmations>=100) {
        control::MessageBox::message( this, "Input", "Please input the change output number in the range from 1 to 100" );
        ui->changeOutputsEdit->setFocus();
        return false;
    }

    QPair<QString,QString> networkArch = wallet::WalletConfig::readNetworkArchFromDataPath(walletDir); // local path as writen in config
    QString runningArc = util::getBuildArch();

    // Just in case. Normally will never be called
    if ( runningArc != networkArch.second ) {
        control::MessageBox::message(nullptr, "Wallet data architecture mismatch",
                                     "Your mwc713 seed at '"+ walletDir +"' was created with "+ networkArch.second+" bits version of the wallet. You are using " + runningArc + " bit version.");
        return false;
    }

    QString network = networkArch.first; // local path as writen in config
    if (network.isEmpty()) {
        // Check if seed file does exist. Import of the data?
        if ( wallet::WalletConfig::doesSeedExist(walletDir) ) {

            dlg::NetworkSelectionDlg nwDlg(this);
            if (nwDlg.exec() != QDialog::Accepted)
                return false;

            network = nwDlg.getNetwork() == state::InitAccount::MWC_NETWORK::MWC_MAIN_NET ? "Mainnet" : "Floonet";

        }
        else
            network = "Mainnet"; // will be redefined later in any case...
    }

    wallet::WalletConfig::saveNetwork2DataPath(walletDir, network, runningArc);

    // So far we are good
    newWalletConfig.setData( network,
            walletDir,
            config::getUseMwcMqS() ? currentWalletConfig.mwcmqDomainEx : mwcmqHost,
            config::getUseMwcMqS() ? mwcmqHost : currentWalletConfig.mwcmqsDomainEx,
            keybasePath,
            prevWalletConfig.mwcNodeURI,
            prevWalletConfig.mwcNodeSecret );
    newSendParams.setData( confirmations, changeOutputs );
    return true;
}


void WalletConfig::on_mwc713directorySelect_clicked()
{
    state::TimeoutLockObject to( state );

    QString basePath = ioutils::getAppDataPath();
    QString dir = QFileDialog::getExistingDirectory(
            nullptr,
            "Select your wallet folder name",
            basePath);
    if (dir.isEmpty())
        return;

    QDir baseDir(basePath);
    QString walletDir = baseDir.relativeFilePath(dir);

    QPair<QString,QString> networkArch = wallet::WalletConfig::readNetworkArchFromDataPath(walletDir); // local path as writen in config
    QString runningArc = util::getBuildArch();

    // Just in case. Normally will never be called
    if ( runningArc != networkArch.second ) {
        control::MessageBox::message(nullptr, "Wallet data architecture mismatch",
                                     "Your mwc713 seed at '"+ walletDir +"' was created with "+ networkArch.second+" bits version of the wallet. You are using " + runningArc + " bit version.");
        return;
    }

    ui->mwc713directoryEdit->setText( walletDir );
    updateButtons();
}

void WalletConfig::on_mwc713directoryEdit_textEdited(const QString &)
{
    updateButtons();
}

void WalletConfig::on_mwcmqHost_textEdited(const QString &)
{
    updateButtons();
}

void WalletConfig::on_keybasePathEdit_textChanged(const QString &)
{
    updateButtons();
}

void WalletConfig::on_keybasePathSelect_clicked()
{
    state::TimeoutLockObject to( state );

    const QStringList appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    QString keybase = QFileDialog::getOpenFileName(this, tr("Keybase binary location"),
                                                   appDirs.isEmpty() ? "" : appDirs[0]);
    if (keybase.isEmpty())
        return;

#ifdef Q_OS_WIN

    // Under the windows we want user select Client, Not a GUI
    // C:\Users\XXXX\AppData\Local\Keybase\keybase.exe        - ok
    // C:\Users\XXXX\AppData\Local\Keybase\Gui\Keybase.exe    - GUI, will not work
    if (keybase.contains("Gui") || keybase.contains("gui") ) {
        if ( control::MessageBox::RETURN_CODE::BTN1 == control::MessageBox::question( this, "Keybase path, Warning",
                       "Wallet requires keybase console client. Seems like you selected keybase GUI that doesn't provide needed functionality. Please double check if console client path was selected.",
                                       "Cancel", "Use this path", true, false ) )
            return;
    }
#endif

    ui->keybasePathEdit->setText( keybase );
    updateButtons();
}

void WalletConfig::on_confirmationNumberEdit_textChanged(const QString &)
{
    updateButtons();
}

void WalletConfig::on_changeOutputsEdit_textEdited(const QString &)
{
    updateButtons();
}

void WalletConfig::on_restoreDefault_clicked()
{
    setValues(defaultWalletConfig.getDataPath(), defaultWalletConfig.keyBasePath, defaultWalletConfig.getMwcMqHostNorm(),
              defaultSendParams.inputConfirmationNumber, defaultSendParams.changeOutputs);

    checkSizeButton( scale2Id(state->getInitGuiScale()) );

    updateButtons();
}

void WalletConfig::on_applyButton_clicked()
{
    wallet::WalletConfig newWalletConfig;
    core::SendCoinsParams newSendParams;

    if ( readInputValue( currentWalletConfig, newWalletConfig, newSendParams ) ) {
        if (! (sendParams == newSendParams)) {
            state->setSendCoinsParams(newSendParams);
            sendParams = newSendParams;
        }

        bool need2updateGuiSize = ( getcheckedSizeButton() != uiScale );

        if (need2updateGuiSize) {
            state->updateGuiScale( id2scale( getcheckedSizeButton() ) );
        }

        if ( ! (currentWalletConfig==newWalletConfig) ) {
            if (state->setWalletConfig(newWalletConfig, need2updateGuiSize)) { // in case of true, we are already dead, don't touch memory and exit!
                return;
            }
        }

        if (need2updateGuiSize) {
            // Restating the wallet
            state->restartMwcQtWallet();
            return;
        }

        updateButtons();
    }
}

// Id match the control names: 1..4
void WalletConfig::checkSizeButton(int szId) {
    ui->fontSz1->setChecked( szId == 1 );
    ui->fontSz2->setChecked( szId == 2 );
    ui->fontSz3->setChecked( szId == 3 );
    ui->fontSz4->setChecked( szId == 4 );

    updateButtons();
}

int WalletConfig::getcheckedSizeButton() const {
    if (ui->fontSz1->isChecked())
        return 1;
    if (ui->fontSz2->isChecked())
        return 2;
    if (ui->fontSz3->isChecked())
        return 3;

    return 4;
}

void WalletConfig::on_fontSz1_clicked()
{
    checkSizeButton(1);
}

void WalletConfig::on_fontSz2_clicked()
{
    checkSizeButton(2);
}

void WalletConfig::on_fontSz3_clicked()
{
    checkSizeButton(3);
}

void WalletConfig::on_fontSz4_clicked()
{
    checkSizeButton(4);
}

}



#include "x_walletconfig_w.h"
#include "ui_x_walletconfig.h"
#include "../state/x_walletconfig.h"
#include "../control/messagebox.h"
#include <QFileInfo>
#include <QHostInfo>
#include "../util/Waiting.h"
#include <QFileDialog>
#include <QStandardPaths>

namespace wnd {

const static QString  MWCMW_DOMAIN_DEFAULT_STR = "default MWC MQ Domain";
const static QString  MWCMW_DOMAIN_DEFAULT_HOST = "mq.mwc.mw";

WalletConfig::WalletConfig(QWidget *parent, state::WalletConfig * _state) :
    core::NavWnd(parent, _state->getStateMachine(), _state->getAppContext() ),
    ui(new Ui::WalletConfig),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Wallet config");

    currentWalletConfig = state->getWalletConfig();
    sendParams = state->getSendCoinsParams();

    setValues();
    updateButtons();
}

WalletConfig::~WalletConfig()
{
    delete ui;
}

void WalletConfig::setValues() {
    ui->mwc713directoryEdit->setText( currentWalletConfig.dataPath );
    ui->keybasePathEdit->setText( keybasePathConfig2InputStr(currentWalletConfig.keyBasePath) );
    ui->mwcmqHost->setText( mwcDomainConfig2InputStr(currentWalletConfig.mwcmqDomain) );

    ui->confirmationNumberEdit->setText( QString::number(sendParams.inputConfirmationNumber) );
    ui->changeOutputsEdit->setText( QString::number(sendParams.changeOutputs) );

    setFocus();
}

void WalletConfig::updateButtons() {
    bool same = true;
    same = same && ui->mwc713directoryEdit->text() == currentWalletConfig.dataPath;
    same = same && keybasePathInputStr2Config( ui->keybasePathEdit->text() ) == currentWalletConfig.keyBasePath;
    same = same && mwcDomainInputStr2Config( ui->mwcmqHost->text() ) == currentWalletConfig.mwcmqDomain;

    same = same && ui->confirmationNumberEdit->text() == QString::number(sendParams.inputConfirmationNumber);
    same = same && ui->changeOutputsEdit->text() == QString::number(sendParams.changeOutputs);

    ui->restoreDefault->setEnabled( !same );
    ui->applyButton->setEnabled( !same );
}


QString WalletConfig::mwcDomainConfig2InputStr(QString mwcDomain) {
    return mwcDomain==MWCMW_DOMAIN_DEFAULT_HOST ? MWCMW_DOMAIN_DEFAULT_STR : mwcDomain;
}

QString WalletConfig::mwcDomainInputStr2Config(QString mwcDomain) {
    return mwcDomain==MWCMW_DOMAIN_DEFAULT_STR ? MWCMW_DOMAIN_DEFAULT_HOST : mwcDomain;
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
bool WalletConfig::readInputValue( wallet::WalletConfig & newWalletConfig, core::SendCoinsParams & newSendParams ) {
    util::Waiting w; // Host verifucation might tale time, what is why waiting here

    // mwc713 directory
    QString walletDir = ui->mwc713directoryEdit->text();
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
    QString keybasePath = keybasePathInputStr2Config( ui->keybasePathEdit->text() );
    res = util::validateMwc713Str(keybasePath);
    if (!res.first) {
        control::MessageBox::message( this, "Input", res.second );
        ui->keybasePathEdit->setFocus();
        return false;
    }

    QString mwcmqHost = mwcDomainInputStr2Config(ui->mwcmqHost->text());
    if (mwcmqHost!=MWCMW_DOMAIN_DEFAULT_HOST) {
        // Checking the host

        QHostInfo host = QHostInfo::fromName(mwcmqHost);
        if (host.error() != QHostInfo::NoError) {
            control::MessageBox::message( this, "Input", "MWC MQ host "+mwcmqHost+" is not reachable.\n" + host.errorString() );
            ui->mwcmqHost->setFocus();
            return false;
        }
    }

    bool ok = false;
    int confirmations = ui->confirmationNumberEdit->text().toInt(&ok);
    if (!ok || confirmations<=0 || confirmations>10) {
        control::MessageBox::message( this, "Input", "Please input the number of confirmations in the range from 1 to 10" );
        ui->confirmationNumberEdit->setFocus();
        return false;
    }

    int changeOutputs = ui->changeOutputsEdit->text().toInt(&ok);
    if (!ok || changeOutputs<=0 || confirmations>=100) {
        control::MessageBox::message( this, "Input", "Please input the change output number in the range from 1 to 100" );
        ui->changeOutputsEdit->setFocus();
        return false;
    }

    // So far we are good
    newWalletConfig.setData(walletDir,
                            mwcmqHost,
                            keybasePath);
    newSendParams.setData( confirmations, changeOutputs );
    return true;
}


void WalletConfig::on_mwc713directorySelect_clicked()
{
    QString basePath = ioutils::getAppDataPath();
    QString dir = QFileDialog::getExistingDirectory(
            nullptr,
            "Select your wallet forder name",
            basePath);
    if (dir.isEmpty())
        return;

    QDir baseDir(basePath);

    ui->mwc713directoryEdit->setText( baseDir.relativeFilePath(dir) );
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

    const QStringList appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    QString keybase = QFileDialog::getOpenFileName(this, tr("Keybase binary location"),
                                                   appDirs.isEmpty() ? "" : appDirs[0]);
    if (keybase.isEmpty())
        return;

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
    setValues();
    updateButtons();
}

void WalletConfig::on_applyButton_clicked()
{
    wallet::WalletConfig newWalletConfig;
    core::SendCoinsParams newSendParams;

    if ( WalletConfig::readInputValue( newWalletConfig, newSendParams ) ) {
        if (! (sendParams == newSendParams)) {
            state->setSendCoinsParams(newSendParams);
            sendParams = newSendParams;
        }

        if ( ! (currentWalletConfig==newWalletConfig) ) {
            if (state->setWalletConfig(newWalletConfig)) { // in case of true, we are already dead, don't touch memory and exit!
                return;
            }
        }
    }
}


}



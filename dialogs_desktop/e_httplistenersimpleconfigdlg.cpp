// Copyright 2021 The MWC Developers
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

#include "e_httplistenersimpleconfigdlg.h"
#include "ui_e_httplistenersimpleconfigdlg.h"
#include "../control_desktop/messagebox.h"
#include <QFileInfo>
#include "../bridge/util_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/x_walletconfig_b.h"

namespace dlg {

HttpListenerSimpleConfigDlg::HttpListenerSimpleConfigDlg(QWidget *parent) :
    control::MwcDialog(parent),
    ui(new Ui::HttpListenerSimpleConfigDlg)
{
    util = new bridge::Util(this);
    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    walletConfig = new bridge::WalletConfig(this);

    ui->setupUi(this);

    foreignApiActive = config->hasForeignApi();
    QString host_port = config->getForeignApiAddress();
    auto hp = host_port.split(":");
    if (hp.size()==2) {
        host = hp[0];
        port = hp[1].toInt();
    }
    if (host.isEmpty())
        host = "127.0.0.1";
    if (port<=0)
        port = 3415;

    tlsCertificateKey = config->getTlsCertificateKey();
    tlsCertificateFile = config->getTlsCertificateFile();

    if (foreignApiActive == false) {
        ui->radioDisableHttp->setChecked(true);
    }
    else if (!tlsCertificateKey.isEmpty() || !tlsCertificateFile.isEmpty() ) {
        ui->radioAdvanced->setChecked(true);
    }
    else if (host == "0.0.0.0") {
        ui->radioInternet->setChecked(true);
    }
    else if (host == "127.0.0.1") {
        ui->radioLocal->setChecked(true);
    }
    else {
        ui->radioAdvanced->setChecked(true);
    }

    ui->portEdit->setText( QString::number(port) );
    ui->listeningAddressEdit->setText( host + ":" + QString::number(port) );
    ui->tlsPrivateKeyEdit->setText(tlsCertificateKey);
    ui->tlsFullchainEdit->setText(tlsCertificateFile);

    updateControlState();
}

void HttpListenerSimpleConfigDlg::updateControlState() {
    if ( ui->radioLocal->isChecked() || ui->radioInternet->isChecked() )
        ui->portHolder->show();
    else
        ui->portHolder->hide();

    if ( ui->radioAdvanced->isChecked() )
        ui->advancedHolder->show();
    else
        ui->advancedHolder->hide();

    QString warnString;
    if (ui->radioDisableHttp->isChecked()) {
        warnString = "<p style=\"line-height:150%;\">Disable foreign API Listener and Tor. The wallet will be able to accept coins through MWC MQS, Files or Slatepacks. Tor, ngrok will not be able to work.</p>";
    }
    else if (ui->radioLocal->isChecked()) {
        warnString = "<p style=\"line-height:150%;\">Foreign API Listener will accept connections from local services like Tor and ngrok. "
                     "If you are not planning to setup port forwarding to receive coins by HTTP to your public IP address, please select this option.</p>";
    }
    else if (ui->radioInternet->isChecked()) {
        warnString = "<p style=\"line-height:150%;\">Foreign API Listener will accept connections from any IP address. The HTTP session will not be encrypted (your traffic will be visible to observer). "
                "You might consider to use this option if you are planning to setup port forwarding to receive coins by HTTP to your public IP address and you don't want to setup TLS for your wallet.</p>";
    }
    else if (ui->radioAdvanced->isChecked()) {
        warnString = "<p style=\"line-height:150%;\">Select this option if you understand how to setup the listener address and certificates for secure TLS connection.</p>";
    }

    ui->explanationmessage->setText(warnString);

    bool getForeignApiActive = false;
    QString getHost = "127.0.0.1";
    int getPort = 3415;
    QString getTlsCertificateKey;
    QString getTlsCertificateFile;

    bool ok = getData(getForeignApiActive, getHost, getPort,getTlsCertificateKey, getTlsCertificateFile).isEmpty();

    ui->applyButton->setEnabled( !(ok && getForeignApiActive == foreignApiActive && getHost == host &&
                                 getPort == port && getTlsCertificateKey == tlsCertificateKey &&
                                getTlsCertificateFile == tlsCertificateFile ));
}

// all params a result.
// return: error message
QString HttpListenerSimpleConfigDlg::getData(bool & getForeignApiActive, QString & getHost, int & getPort,
                                          QString & getTlsCertificateKey, QString & getTlsCertificateFile) {
    getForeignApiActive = foreignApiActive;
    getHost = host;
    getPort = port;
    getTlsCertificateKey = "";
    getTlsCertificateFile = "";

    if (ui->radioDisableHttp->isChecked()) {
        getForeignApiActive = false;
        return "";
    }
    else if (ui->radioLocal->isChecked() || ui->radioInternet->isChecked()) {
        getForeignApiActive = true;
        if (ui->radioLocal->isChecked())
            getHost = "127.0.0.1";
        else
            getHost = "0.0.0.0";

        bool ok = false;
        getPort = ui->portEdit->text().toInt(&ok);
        if (ok && getPort>0)
            return "";
        else {
            ui->portEdit->setFocus();
            return "Please specify valid port";
        }
    }
    else if (ui->radioAdvanced->isChecked()) {
        getForeignApiActive = true;
        QString hostPort = ui->listeningAddressEdit->text();
        auto hp = hostPort.split(":");
        if (hp.size()!=2) {
            ui->listeningAddressEdit->setFocus();
            return "Please specify valid listening address.";
        }
        getHost = hp[0];
        bool ok = false;
        getPort = hp[1].toInt(&ok);
        if (getHost.isEmpty() || !ok || getPort<=0) {
            ui->listeningAddressEdit->setFocus();
            return "Please specify valid listening address.";
        }

        getTlsCertificateKey = ui->tlsPrivateKeyEdit->text();
        getTlsCertificateFile = ui->tlsFullchainEdit->text();
        if ( getTlsCertificateKey.isEmpty() != getTlsCertificateFile.isEmpty() ) {
            return "Please specify both tls certificate files or none of them.";
        }
        return "";
    }

    Q_ASSERT(false);
    getForeignApiActive = false;
    return "";
}


HttpListenerSimpleConfigDlg::~HttpListenerSimpleConfigDlg()
{
    delete ui;
}

void HttpListenerSimpleConfigDlg::on_radioDisableHttp_clicked()
{
    updateControlState();
}

void HttpListenerSimpleConfigDlg::on_radioLocal_clicked()
{
    updateControlState();
}

void HttpListenerSimpleConfigDlg::on_radioInternet_clicked()
{
    updateControlState();
}

void HttpListenerSimpleConfigDlg::on_radioAdvanced_clicked()
{
    updateControlState();
}

void HttpListenerSimpleConfigDlg::on_selectPrivKeyButton_clicked()
{
    QString fileName = util->getOpenFileName("Select certificate private key",
                                                    "",
                                                    "Certificate key pem (*.pem);;All files (*.*)");
    if (fileName.isEmpty())
        return;
    ui->tlsPrivateKeyEdit->setText(fileName);
}

void HttpListenerSimpleConfigDlg::on_selectFullchainButton_clicked()
{
    QString fileName = util->getOpenFileName("Select fullchain certificate",
                                                    "",
                                                    "Fullchain certificate pem (*.pem);;All files (*.*)");

    if (fileName.isEmpty())
        return;

    ui->tlsFullchainEdit->setText(fileName);
}

void HttpListenerSimpleConfigDlg::on_cancelButton_clicked()
{
    reject();
}

void HttpListenerSimpleConfigDlg::on_applyButton_clicked()
{
    bool getForeignApiActive = false;
    QString getHost = "127.0.0.1";
    int getPort = 3415;
    QString getTlsCertificateKey;
    QString getTlsCertificateFile;

    QString error = getData(getForeignApiActive, getHost, getPort,getTlsCertificateKey, getTlsCertificateFile);
    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Incorrect Input", error);
        // set focus suppose to be done by getData
        return;
    }

    bool showMsg = false;
    if (walletConfig->getAutoStartTorEnabled()) {
        if (! getForeignApiActive || (!getTlsCertificateFile.isEmpty() && !getTlsCertificateKey.isEmpty()) ) {
            showMsg = true;
            if ( core::WndManager::RETURN_CODE::BTN1 == control::MessageBox::questionText( this, "Tor Listener",
                                               "Tor requires Foreign API not use TLS. Your configuration is different. "
                                               "Do you want to apply your configuration and disable autostart Tor listener?",
                                               "Cancel", "Disable Tor",
                                               "Drop my Foreign API changes in order to keep Tor",
                                               "Disable autostart for Tor listener and continue",
                                               false, true ) ) {
                return;
            }
            walletConfig->updateAutoStartTorEnabled(false); // disable Tor starting
        }
    }

    if (!showMsg) {
        showMsg = true;
        if ( core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionText( this, "Warning",
                "Foreign API configuration require to relogin. If mwc713 will not be able to start with those settings, they will be reverted back to 'Local Services'.",
                "Cancel", "Continue",
                "Drop my Foreign API changes",
                "Aplly changes and relogin into this wallet",
                false, true ) ) {

            // apply settings...
            config->saveForeignApiConfig(getForeignApiActive, getHost + ":" + QString::number(getPort), getTlsCertificateFile, getTlsCertificateKey);
            accept();
        }
    }
}

void HttpListenerSimpleConfigDlg::on_listeningAddressEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);
    updateControlState();
}

void HttpListenerSimpleConfigDlg::on_tlsPrivateKeyEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    updateControlState();
}

void HttpListenerSimpleConfigDlg::on_tlsFullchainEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    updateControlState();
}

void HttpListenerSimpleConfigDlg::on_portEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);
    updateControlState();
}

}

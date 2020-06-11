// Copyright 2020 The MWC Developers
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

#include "e_httplistenerconfigdlg.h"
#include "ui_e_httplistenerconfigdlg.h"
#include <QFileDialog>
#include "../control_desktop/messagebox.h"
#include <QFileInfo>
#include "../bridge/util_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"

namespace dlg {

HttpListenerConfigDlg::HttpListenerConfigDlg(QWidget *parent) :
    control::MwcDialog(parent),
    ui(new Ui::HttpListenerConfigDlg)
{
    util = new bridge::Util(this);
    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);

    ui->setupUi(this);

    ui->activateRestApi->setChecked(config->hasForeignApi());
    QString foreignApiAddress = config->getForeignApiAddress();
    ui->listeningAddressEdit->setText(foreignApiAddress.isEmpty() ? "127.0.0.1:3415" : foreignApiAddress);

    QString foreignApiSecret = config->getForeignApiSecret();
    ui->useBasicAutorization->setChecked( !foreignApiSecret.isEmpty() );
    ui->apiSecretEdit->setText( foreignApiSecret.isEmpty() ? util->generateApiSecret(20)  : foreignApiSecret );

    ui->useTlsCheck->setChecked( config->hasTls() );
    ui->tlsPrivateKeyEdit->setText( config->getTlsCertificateKey() );
    ui->tlsFullchainEdit->setText( config->getTlsCertificateFile() );

    updateControlState();
}

HttpListenerConfigDlg::~HttpListenerConfigDlg()
{
    delete ui;
}

void HttpListenerConfigDlg::on_selectPrivKeyButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select certificate private key"),
                                                    "",
                                                    tr("Certificate key pem (*.pem);;All files (*.*)"));

    if (fileName.length()==0)
        return;

    ui->tlsPrivateKeyEdit->setText(fileName);
}

void HttpListenerConfigDlg::on_selectFullchainButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select fullchain certificate"),
                                                    "",
                                                    tr("Fullchain certificate pem (*.pem);;All files (*.*)"));

    if (fileName.length()==0)
        return;

    ui->tlsFullchainEdit->setText(fileName);
}

void HttpListenerConfigDlg::on_resetButton_clicked()
{
    ui->activateRestApi->setChecked(false);
    ui->listeningAddressEdit->setText("127.0.0.1:3415");

    ui->useBasicAutorization->setChecked( true );
    ui->apiSecretEdit->setText(util->generateApiSecret(20) );

    updateControlState();
}

void HttpListenerConfigDlg::on_activateRestApi_stateChanged(int check)
{
    Q_UNUSED(check)
    updateControlState();
}

void HttpListenerConfigDlg::on_useBasicAutorization_stateChanged(int check)
{
    Q_UNUSED(check)
    updateControlState();
}

void HttpListenerConfigDlg::on_useTlsCheck_stateChanged(int check)
{
    Q_UNUSED(check)
    updateControlState();
}

void HttpListenerConfigDlg::on_generateSecretButton_clicked()
{
    ui->apiSecretEdit->setText( util->generateApiSecret(20) );
}

void HttpListenerConfigDlg::updateControlState() {
    bool api = ui->activateRestApi->isChecked();
    bool auth = ui->useBasicAutorization->isChecked();
    bool tls = ui->useTlsCheck->isChecked();

    ui->listeningAddressEdit->setEnabled(api);
    ui->listeningAddress_label->setEnabled(api);
    ui->useBasicAutorization->setEnabled(api);
    ui->apiSecretEdit->setEnabled(api && auth);
    ui->apiSecret_label->setEnabled(api && auth);
    ui->generateSecretButton->setEnabled(api && auth);

    ui->useTlsCheck->setEnabled(api);
    ui->tlsPrivateKeyEdit->setEnabled(api && tls);
    ui->tlsPrivateKey_label->setEnabled(api && tls);
    ui->tlsFullchainEdit->setEnabled(api && tls);
    ui->tlsFullchain_label->setEnabled(api && tls);
    ui->selectPrivKeyButton->setEnabled(api && tls);
    ui->selectFullchainButton->setEnabled(api && tls);

    ui->resetButton->setEnabled(api);
}

void HttpListenerConfigDlg::on_cancelButton_clicked()
{
    reject();
}

void HttpListenerConfigDlg::on_applyButton_clicked()
{
    bool foreignApi = false;
    QString foreignApiAddress = "";
    QString foreignApiSecret = "";
    QString tlsCertificateFile = "";
    QString tlsCertificateKey = "";

    // Let's check parameters that we set up so far...
    if (ui->activateRestApi->isChecked()) {
        foreignApi = true;

        QString listenAddress = ui->listeningAddressEdit->text();
        if (listenAddress.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", "Please specify listening foreign API address" );
            ui->listeningAddressEdit->setFocus();
            return;
        }

        // Need to check the inputs
        QPair <bool, QString> valRes = util::validateMwc713Str( listenAddress );
        if (!valRes.first) {
            control::MessageBox::messageText(this, "Listening Address", valRes.second );
            ui->listeningAddressEdit->setFocus();
            return;
        }

        // Check if port is here
        int semicolonPos = listenAddress.lastIndexOf(':');
        if (semicolonPos<=0) {
            control::MessageBox::messageText(this, "Incorrect Input", "Please specify port for listening foreign API address" );
            ui->listeningAddressEdit->setFocus();
            return;
        }
        int digits = 0;
        int nonDigits = 0;
        for ( int r=semicolonPos+1; r<listenAddress.length(); r++ ) {
            if ( listenAddress[r].isDigit() )
                digits++;
            else
                nonDigits++;
        }

        if (digits==0 || nonDigits!=0) {
            control::MessageBox::messageText(this, "Incorrect Input", "Please specify port for listening foreign API address" );
            ui->listeningAddressEdit->setFocus();
            return;
        }

        foreignApiAddress = listenAddress;

        if (ui->useBasicAutorization->isChecked() ) {
            QString apiSecret = ui->apiSecretEdit->text();
            if (apiSecret.isEmpty()) {
                control::MessageBox::messageText(this, "Incorrect Input", "Please API secret value" );
                ui->apiSecretEdit->setFocus();
                return;
            }

            // Need to check the inputs
            QPair <bool, QString> valRes = util::validateMwc713Str( apiSecret );
            if (!valRes.first) {
                control::MessageBox::messageText(this, "API Secret", valRes.second );
                ui->apiSecretEdit->setFocus();
                return;
            }

            foreignApiSecret = apiSecret;
        }
        else {
            foreignApiSecret = "";
        }

        if (ui->useTlsCheck->isChecked()) {

            QString privKey = ui->tlsPrivateKeyEdit->text();
            QString chain = ui->tlsFullchainEdit->text();

            // Need to check the inputs
            QPair <bool, QString> valRes = util::validateMwc713Str( privKey );
            if (!valRes.first) {
                control::MessageBox::messageText(this, "file name", valRes.second );
                ui->tlsPrivateKeyEdit->setFocus();
                return;
            }

            // Need to check the inputs
            valRes = util::validateMwc713Str( chain );
            if (!valRes.first) {
                control::MessageBox::messageText(this, "file name", valRes.second );
                ui->tlsFullchainEdit->setFocus();
                return;
            }


            if ( privKey.isEmpty() || !QFileInfo::exists(privKey) ) {
                control::MessageBox::messageText(this, "Incorrect Input", "Please tls private key file" );
                ui->tlsPrivateKeyEdit->setFocus();
                return;
            }

            if ( chain.isEmpty() || !QFileInfo::exists(chain) ) {
                control::MessageBox::messageText(this, "Incorrect Input", "Please tls fullchain certificate file" );
                ui->tlsFullchainEdit->setFocus();
                return;
            }

            tlsCertificateFile = chain;
            tlsCertificateKey = privKey;
        }
        else {
            tlsCertificateFile = "";
            tlsCertificateKey = "";
        }

    }
    else {
        foreignApi = false;
        foreignApiAddress = "";
        foreignApiSecret = "";
        tlsCertificateFile = "";
        tlsCertificateKey = "";
    }

    QString message;
    if (foreignApi) {
        if (foreignApiSecret.isEmpty())
            message += "without any authorization";

        if (tlsCertificateFile.isEmpty() || tlsCertificateKey.isEmpty()) {
            if (!message.isEmpty())
                message += " and ";

            message += "with non secure HTTP connection. Please consider to setup TLS certificates for your security.";
        }

        if (!message.isEmpty()) {
            message = "Warning. Your wallet has activated foreign API " + message + "\n\n";
        }
    }

    if ( core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionText( this, "Warning",
            message + "Foreign API configuration require to relogin. If mwc713 will not be able to start with those settings, they will be reverted back to default.",
            "Cancel", "Continue", false, true ) ) {

        // apply settings...
        config->saveForeignApiConfig(foreignApi,foreignApiAddress, foreignApiSecret, tlsCertificateFile, tlsCertificateKey);
        accept();
    }
}


}


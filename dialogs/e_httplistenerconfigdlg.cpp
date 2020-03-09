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

#include "dialogs/e_httplistenerconfigdlg.h"
#include "ui_e_httplistenerconfigdlg.h"
#include "../util/Generator.h"
#include <QFileDialog>
#include "../control/messagebox.h"
#include <QFileInfo>

namespace dlg {

HttpListenerConfigDlg::HttpListenerConfigDlg(QWidget *parent, const wallet::WalletConfig &_config) :
    control::MwcDialog(parent),
    ui(new Ui::HttpListenerConfigDlg),
    config(_config)
{
    ui->setupUi(this);

    ui->activateRestApi->setChecked(config.hasForeignApi());
    ui->listeningAddressEdit->setText(config.foreignApiAddress.isEmpty() ? "0.0.0.0:3415" : config.foreignApiAddress);

    ui->useBasicAutorization->setChecked( !config.foreignApiSecret.isEmpty() );
    ui->apiSecretEdit->setText( config.foreignApiSecret.isEmpty() ? util::generateSecret(20)  : config.foreignApiSecret );

    ui->useTlsCheck->setChecked( config.hasTls() );
    ui->tlsPrivateKeyEdit->setText( config.tlsCertificateKey );
    ui->tlsFullchainEdit->setText( config.tlsCertificateFile );

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
    ui->listeningAddressEdit->setText("0.0.0.0:3415");

    ui->useBasicAutorization->setChecked( true );
    ui->apiSecretEdit->setText(util::generateSecret(20) );

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
    ui->apiSecretEdit->setText( util::generateSecret(20) );
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
    // Let's check parameters that we set up so far...
    if (ui->activateRestApi->isChecked()) {
        config.foreignApi = true;

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

        config.foreignApiAddress = listenAddress;

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

            config.foreignApiSecret = apiSecret;
        }
        else {
            config.foreignApiSecret = "";
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

            config.tlsCertificateFile = chain;
            config.tlsCertificateKey = privKey;
        }
        else {
            config.tlsCertificateFile = "";
            config.tlsCertificateKey = "";
        }

    }
    else {
        config.foreignApi = false;
        config.foreignApiAddress = "";
        config.foreignApiSecret = "";
        config.tlsCertificateFile = "";
        config.tlsCertificateKey = "";
    }

    if ( control::MessageBox::RETURN_CODE::BTN2 == control::MessageBox::questionText( this, "Warning",
            "Foreign API configuration require to relogin. If mwc713 will not be able to start with those settings, they will be reverted back to default.",
            "Cancel", "Continue", false, true ) ) {
        accept();
    }
}


}


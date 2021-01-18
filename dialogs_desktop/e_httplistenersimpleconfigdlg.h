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

#ifndef E_HTTPLISTENERSIMPLECONFIGDLG_H
#define E_HTTPLISTENERSIMPLECONFIGDLG_H

#include "../control_desktop/mwcdialog.h"
#include "../wallet/wallet.h"

namespace Ui {
class HttpListenerSimpleConfigDlg;
}

namespace bridge {
class Util;
class Wallet;
class Config;
class WalletConfig;
}

namespace dlg {

class HttpListenerSimpleConfigDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit HttpListenerSimpleConfigDlg(QWidget *parent = nullptr);
    ~HttpListenerSimpleConfigDlg();

private slots:
    void on_radioDisableHttp_clicked();
    void on_radioLocal_clicked();
    void on_radioInternet_clicked();
    void on_radioAdvanced_clicked();
    void on_selectPrivKeyButton_clicked();
    void on_selectFullchainButton_clicked();
    void on_cancelButton_clicked();
    void on_applyButton_clicked();

    void on_listeningAddressEdit_textEdited(const QString &arg1);

    void on_tlsPrivateKeyEdit_textChanged(const QString &arg1);

    void on_tlsFullchainEdit_textChanged(const QString &arg1);

    void on_portEdit_textEdited(const QString &arg1);

private:
    void updateControlState();

    // all params a result.
    // return is error message
    QString getData(bool & getForeignApiActive, QString & getHost, int & getPort,
                 QString & getTlsCertificateKey, QString & getTlsCertificateFile);

private:
    Ui::HttpListenerSimpleConfigDlg *ui;

    bridge::Util * util = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Config * config = nullptr;
    bridge::WalletConfig * walletConfig = nullptr;

    bool foreignApiActive = false;
    QString host = "127.0.0.1";
    int port = 3415;
    QString tlsCertificateKey;
    QString tlsCertificateFile;
};

}

#endif // E_HTTPLISTENERSIMPLECONFIGDLG_H

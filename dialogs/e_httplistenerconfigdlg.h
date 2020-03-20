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

#ifndef E_HTTPLISTENERCONFIGDLG_H
#define E_HTTPLISTENERCONFIGDLG_H

#include "../control/mwcdialog.h"
#include "../wallet/wallet.h"

namespace Ui {
class HttpListenerConfigDlg;
}

namespace dlg {

class HttpListenerConfigDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit HttpListenerConfigDlg(QWidget *parent, const wallet::WalletConfig &config);
    ~HttpListenerConfigDlg();

    const wallet::WalletConfig & getConfig() const {return config;}

private slots:
    void on_selectPrivKeyButton_clicked();

    void on_selectFullchainButton_clicked();

    void on_resetButton_clicked();

    void on_cancelButton_clicked();

    void on_applyButton_clicked();

    void on_activateRestApi_stateChanged(int check);

    void on_useBasicAutorization_stateChanged(int check);

    void on_useTlsCheck_stateChanged(int check);

    void on_generateSecretButton_clicked();

private:
    void updateControlState();

private:
    Ui::HttpListenerConfigDlg *ui;
    wallet::WalletConfig config;
};

}

#endif // E_HTTPLISTENERCONFIGDLG_H

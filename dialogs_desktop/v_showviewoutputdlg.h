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

#ifndef E_SHOWVIEWOUTPUTDLG_H
#define E_SHOWVIEWOUTPUTDLG_H

#include "../control_desktop/mwcdialog.h"
#include "../wallet/wallet.h"

namespace Ui {
class ShowViewOutputDlg;
}

namespace bridge {
class Config;
class Wallet;
class Util;
}

namespace dlg {

class ShowViewOutputDlg : public control::MwcDialog {
Q_OBJECT
public:
    explicit ShowViewOutputDlg(QWidget *parent, const wallet::WalletOutput &output);

    ~ShowViewOutputDlg();

    bool isLocked() const {return locked;}
    QString getResultOutputNote() const {return newOutputNote;}

private slots:
    void on_viewOutput_clicked();
    void on_okButton_clicked();

private:
    void updateButtons(bool showOutputEditButtons);

private:
    Ui::ShowViewOutputDlg *ui;

    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Util * util = nullptr;

    QString blockExplorerUrl;
    QString commitment;
    QString newOutputNote;
    bool    locked;
};

}

#endif // E_SHOWOUTPUTDLG_H

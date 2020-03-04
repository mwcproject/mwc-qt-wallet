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

#ifndef E_SHOWOUTPUTDLG_H
#define E_SHOWOUTPUTDLG_H

#include "../control/mwcdialog.h"

namespace Ui {
class ShowOutputDlg;
}

namespace wallet {
struct WalletOutput;
struct WalletConfig;
}

namespace dlg {

class ShowOutputDlg : public control::MwcDialog {
Q_OBJECT
public:
    explicit ShowOutputDlg(QWidget *parent, const wallet::WalletOutput &output, const wallet::WalletConfig &config);

    ~ShowOutputDlg();

private slots:

    void on_viewOutput_clicked();

    void on_pushButton_clicked();

private:
    Ui::ShowOutputDlg *ui;

    QString blockExplorerUrl;
    QString commitment;
};

}

#endif // E_SHOWOUTPUTDLG_H

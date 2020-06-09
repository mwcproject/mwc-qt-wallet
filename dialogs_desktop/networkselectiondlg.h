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

#ifndef NETWORKSELECTIONDLG_H
#define NETWORKSELECTIONDLG_H

#include "../control_desktop/mwcdialog.h"
#include "../state/a_initaccount.h"

namespace Ui {
class NetworkSelectionDlg;
}

namespace dlg {

class NetworkSelectionDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit NetworkSelectionDlg(QWidget *parent = nullptr);
    ~NetworkSelectionDlg();

    state::InitAccount::MWC_NETWORK getNetwork() const {return network;}

private slots:
    void on_cancelButton_clicked();

    void on_submitButton_clicked();

private:
    Ui::NetworkSelectionDlg *ui;
    state::InitAccount::MWC_NETWORK network = state::InitAccount::MWC_MAIN_NET;
};

}

#endif // NETWORKSELECTIONDLG_H

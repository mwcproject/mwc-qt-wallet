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

#ifndef X_WALLETINSTANCES_H
#define X_WALLETINSTANCES_H

#include "../control/mwcdialog.h"

namespace Ui {
class WalletInstances;
}

namespace state {
class WalletConfig;
}

namespace dlg {

class WalletInstances : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit WalletInstances(QWidget *parent, state::WalletConfig * state);
    ~WalletInstances();

private slots:
    void on_mwc713directorySelect_clicked();

    void on_cancelButton_clicked();

    void on_applyButton_clicked();

private:
    Ui::WalletInstances *ui;
    state::WalletConfig * state;
};

}


#endif // X_WALLETINSTANCES_H

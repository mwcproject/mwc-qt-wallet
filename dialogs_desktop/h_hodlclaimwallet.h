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

#ifndef H_HODLCLAIMWALLET_H
#define H_HODLCLAIMWALLET_H

#include "../control/mwcdialog.h"

namespace Ui {
class HodlClaimWallet;
}

namespace dlg {

class HodlClaimWallet : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit HodlClaimWallet(QWidget *parent);
    ~HodlClaimWallet();

    // Empty value - this wallet
    QString getColdWalletPublicKeyHash() const {return coldWalletPubKeyHash;}

private slots:
    void on_claimForThisWalletCheck_stateChanged(int state);
    void on_cancelButton_clicked();
    void on_continueButton_clicked();

private:
    Ui::HodlClaimWallet *ui;
    QString coldWalletPubKeyHash;
};

}

#endif // H_HODLCLAIMWALLET_H

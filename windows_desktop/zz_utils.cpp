// Copyright 2025 The MWC Developers
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

#include "zz_utils.h"

namespace wnd {

// Utility methods for desktop UI.

void updateAccountsData(bridge::Wallet * wallet, control::MwcComboBox *accountComboBox, bool hide_integrity_account, bool receivePage) {
    QVector<QString> accounts = wallet->getWalletBalance(true,true,false,true);
    QString selectedAccount;
    if (receivePage)
        selectedAccount = wallet->getReceiveAccountPath();
    else
        selectedAccount = wallet->getCurrentAccountId();

    Q_ASSERT(accounts.size()>=3);
    Q_ASSERT(accounts.size()%3==0);

    int selectedAccIdx = 0;

    accountComboBox->clear();

    int idx = 0;
    for (int i=2; i<accounts.size(); i+=3) {
        if ( hide_integrity_account && accounts[i-2] == "integrity")
            continue;

        if (accounts[i-1] == selectedAccount)
            selectedAccIdx = idx;

        // Data is <Path>,<Name>
        // This a decoded back at accountComboData2AccountPath
        accountComboBox->addItem(accounts[i],  accounts[i-1] + "," + accounts[i-2] );
        idx++;
    }
    accountComboBox->setCurrentIndex(selectedAccIdx);
}

// Convert conbo data back into the QPair<account name, account path>
QPair<QString, QString> accountComboData2AccountPath(QString data) {
    int idx = data.indexOf(',');
    if (idx<0)
        return QPair<QString, QString>("","");

    Q_ASSERT(idx>1 && idx<data.length()-1);
    return QPair<QString, QString>( data.mid(idx+1), data.left(idx) );
}


}

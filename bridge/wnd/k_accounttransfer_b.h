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

#ifndef MWC_QT_WALLET_K_ACCOUNTTRANSFER_B_H
#define MWC_QT_WALLET_K_ACCOUNTTRANSFER_B_H

#include <QObject>

namespace bridge {

class AccountTransfer : public QObject{
Q_OBJECT
public:
    explicit AccountTransfer(QObject * parent = nullptr);
    ~AccountTransfer();

    void showTransferResults(bool ok, QString errMsg);
    void updateAccounts();
    void hideProgress();

    // Initiate Transfer funds from account to account process.
    Q_INVOKABLE bool transferFunds(QString from, QString to, QString sendAmount);

    // Back button was pressed.
    Q_INVOKABLE void goBack();
signals:
    void sgnShowTransferResults(bool ok, QString errMsg);
    void sgnUpdateAccounts();
    void sgnHideProgress();
};

}

#endif //MWC_QT_WALLET_K_ACCOUNTTRANSFER_B_H

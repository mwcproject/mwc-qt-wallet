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

#ifndef MWC_QT_WALLET_K_ACCOUNTS_B_H
#define MWC_QT_WALLET_K_ACCOUNTS_B_H

#include <QObject>
#include <QVector>

namespace bridge {

class Accounts : public QObject {
Q_OBJECT
public:
    explicit Accounts(QObject * parent = nullptr);
    ~Accounts();

    // Return data ready to insert into the Accounts table
    // Data returned in series of 6 (isHodl==true) or 5 (isHodl==false)
    // [accountName, Spendable, Awaiting, Locked, Total, [in Hodl]]
    Q_INVOKABLE QVector<QString> getAccountsBalancesToShow(bool inHodl);

    // Validate new account name. Return Empty string for ok.
    // Otherwise return error message
    Q_INVOKABLE QString validateNewAccountName(QString accName);

    // Check if can delete the account
    Q_INVOKABLE bool canDeleteAccount(QString accountName);

    // Switch to transfer funds page
    Q_INVOKABLE void doTransferFunds();

    // Delete account (wallet can do rename only with prefix that will be hidden after)
    // Check Signal from Wallet: sgnAccountRenamed(bool success, QString errorMessage);
    Q_INVOKABLE void deleteAccount( QString accountName );

};

}

#endif //MWC_QT_WALLET_K_ACCOUNTS_B_H

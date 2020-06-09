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

#ifndef MWC_QT_WALLET_INITACCOUNT_B
#define MWC_QT_WALLET_INITACCOUNT_B

#include <QObject>

namespace bridge {

class InitAccount : public QObject {
Q_OBJECT
public:
    explicit InitAccount(QObject *parent = nullptr);
    ~InitAccount();

    // Set Password for a new account (First step of account creation)
    Q_INVOKABLE void setPassword(QString password);

    // Cancel whatever init account you have and return to the starting page
    Q_INVOKABLE void cancelInitAccount();

    // Create a wallet form the seed
    Q_INVOKABLE void createWalletWithSeed(QVector<QString> seed);

    // newWalletChoice: see values at state::InitAccount::NEW_WALLET_CHOICE
    // network: see values at state::InitAccount::MWC_NETWORK
    Q_INVOKABLE void submitWalletCreateChoices(int newWalletChoice, int network);

};

}

#endif //MWC_QT_WALLET_INITACCOUNT_H

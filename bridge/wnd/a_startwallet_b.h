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

#ifndef MWC_QT_WALLET_A_STARTWALLET_B_H
#define MWC_QT_WALLET_A_STARTWALLET_B_H

#include <QObject>

namespace bridge {

class StartWallet : public QObject {
    Q_OBJECT
public:
    explicit StartWallet(QObject *parent = nullptr);
    ~StartWallet();

    // Create a new Wallet Instance
    // path - wallet data location or empty for the next available_path (new wallet case only)
    // restoreWallet - true if the next new wallet will be restore from the seed.
    Q_INVOKABLE void createNewWalletInstance(QString path, bool restoreWallet);
private:
};

}


#endif //MWC_QT_WALLET_A_STARTWALLET_B_H

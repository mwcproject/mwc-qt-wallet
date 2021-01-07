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

#ifndef MWC_QT_WALLET_G_SEND_B_H
#define MWC_QT_WALLET_G_SEND_B_H

#include <QObject>
#include "../../wallet/wallet.h"

namespace bridge {

enum SEND_SELECTED_METHOD {
    ONLINE_ID = 1, FILE_ID = 2, SLATEPACK_ID = 3
};

class Send : public QObject {
Q_OBJECT
public:
    explicit Send(QObject * parent = nullptr);
    ~Send();

    void updateAccountBalance( QVector<wallet::AccountInfo> accountInfo, const QString & selectedAccount );

    void showSendResult( bool success, QString message );

    // Process sept 1 send request.  sendAmount is a value as user input it
    // sendSelectedMethod values (SEND_SELECTED_METHOD):
    //   ONLINE_ID = 1, FILE_ID = 2, SLATEPACK_ID = 2
    // return code:
    //   0 - ok
    //   1 - account error
    //   2 - amount error
    Q_INVOKABLE int initialSendSelection( int sendSelectedMethod, QString account, QString sendAmount );

    // Check if Node is healthy to do send.
    Q_INVOKABLE bool isNodeHealthy();

    // Handle whole workflow to send offline
    // apiSecret - is not needed. Pass empty String
    // return true if some long process was started.
    Q_INVOKABLE bool sendMwcOffline( QString account, QString amountNano, QString message, bool isSlatepack, bool isLockLater, QString slatepackRecipientAddress);

    // Handle whole workflow to send online
    // return true if some long process was started.
    Q_INVOKABLE bool sendMwcOnline( QString account, QString amountNano, QString address, QString apiSecret, QString message);

    // Return string representing spendable amount for account minus transaction fee
    // Returns "All" if the amount cannot be calculated
    Q_INVOKABLE QString getSpendAllAmount( QString account);

signals:
    void sgnShowSendResult( bool success, QString message );
};

}

#endif //MWC_QT_WALLET_G_SEND_B_H

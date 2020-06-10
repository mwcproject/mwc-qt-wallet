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

#ifndef MWC_QT_WALLET_ACCOUNTTRANSFER_H
#define MWC_QT_WALLET_ACCOUNTTRANSFER_H

#include "state.h"
#include <QObject>

namespace state {

class AccountTransfer : public QObject, public State {
Q_OBJECT
public:
    AccountTransfer( StateContext * context);
    virtual ~AccountTransfer() override;

    // nanoCoins < 0 - all funds
    // Note, accountInfo must be passed (or used inside method) by value because event loop can be run inside that method
    // return true if longterm process was started...
    bool transferFunds(const QString & accountFrom,
                       const QString & accountTo,
                       const QString & mwc );

    void goBack();

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "accounts.html";}

private slots:
    // set receive account name results
    void onSetReceiveAccount( bool ok, QString AccountOrMessage );
    void onSend( bool success, QStringList errors, QString address, int64_t txid, QString slate, QString mwc );
    void onWalletBalanceUpdated();

private:
    int transferState = -1;

    QString recieveAccount;
    // Single transfer context
    QString myAddress;
    QString trAccountFrom;
    QString trAccountTo;
    int64_t trNanoCoins = 0;
    QStringList outputs2use;
};

}

#endif //MWC_QT_WALLET_ACCOUNTTRANSFER_H

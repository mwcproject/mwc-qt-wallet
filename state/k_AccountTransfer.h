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
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace wnd {
class AccountTransfer;
}


namespace state {

class AccountTransfer : public QObject, public State {
Q_OBJECT
public:
    AccountTransfer( StateContext * context);
    virtual ~AccountTransfer() override;

    void wndDeleted(wnd::AccountTransfer * w) { if(w==wnd) wnd = nullptr; }

    // get balance for current account
    QVector<wallet::AccountInfo> getWalletBalance();

    // nanoCoins < 0 - all funds
    // Note, accountInfo must be passed (or used inside method) by value because event loop can be run inside that method
    void transferFunds(const wallet::AccountInfo accountFrom,
                       const wallet::AccountInfo accountTo,
                       int64_t nanoCoins );

    void goBack();

    core::SendCoinsParams getSendCoinsParams();
    void updateSendCoinsParams(const core::SendCoinsParams &params);

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "accounts.html";}

private slots:
    // set receive account name results
    void onSetReceiveAccount( bool ok, QString AccountOrMessage );
    void onSend( bool success, QStringList errors, QString address, int64_t txid, QString slate );
    void onSlateSendTo( QString slate, QString mwc, QString sendAddr );
    void onSlateFinalized( QString slate );
    void onWalletBalanceUpdated();

private:
    wnd::AccountTransfer * wnd = nullptr;
    int transferState = -1;

    // Single transfer context
    QString myAddress;
    wallet::AccountInfo trAccountFrom;
    wallet::AccountInfo trAccountTo;
    int64_t trNanoCoins = 0;
    QString trSlate;
    QStringList outputs2use;
};

}

#endif //MWC_QT_WALLET_ACCOUNTTRANSFER_H

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

#ifndef MWC_QT_WALLET_SEND_H
#define MWC_QT_WALLET_SEND_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../util/address.h"
#include "../bridge/wnd/g_send_b.h"
#include <QSet>

class QtAndroidService;

namespace state {

QString generateAmountErrorMsg(qint64 mwcAmount, const wallet::AccountInfo &acc, const core::SendCoinsParams &sendParams);

class Send  : public QObject, public State {
    Q_OBJECT
public:
    Send( StateContext * context);
    virtual ~Send() override;

    // Process sept 1 send request.  sendAmount is a value as user input it
    // return code:
    //   0 - ok
    //   1 - account error
    //   2 - amount error
    int initialSendSelection( bridge::SEND_SELECTED_METHOD sendSelectedMethod, QString accountPath, QString sendAmount, bool gotoNextPage );
    // Data that was collected by initialSendSelection. Can be used if gotoNextPage is false
    qint64 getTmpAmount() const {return tmpAmount;}
    QString getTmpAccountPath() const {return tmpAccountPath;}

    // Handle whole workflow to send offline
    // return true if some long process was started.
    bool sendMwcOffline( const QString & account, const QString & accountPath, qint64 amount, const QString & message, bool isLockLater, const QString & slatepackRecipientAddress );

    // Handle whole workflow to send online
    // return true if some long process was started.
    bool sendMwcOnline( const QString & account, const QString & accountPath, qint64 amount, QString address, const QString & message );

    // Returns the amount of coins, minus the transaction fee, which can be spent for this account
    QString getSpendAllAmount(QString accountPath);

protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override;
    virtual QString getHelpDocName() override;

private slots:
    void sendRespond(bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag);

private:
    void switchToStartingWindow();

    QString getAccountPathByName(const QString account, bool showErrMessage);
private:
    bool atSendInitialPage = true;

    // temp buffered data
    qint64 tmpAmount = 0;
    QString tmpAccountPath;
};

}

#endif

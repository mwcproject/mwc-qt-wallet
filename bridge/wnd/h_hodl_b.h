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

#ifndef MWC_QT_WALLET_H_HODL_B_H
#define MWC_QT_WALLET_H_HODL_B_H

#include <QObject>

namespace bridge {

class Hodl : public QObject {
Q_OBJECT
public:
    explicit Hodl(QObject * parent = nullptr);
    ~Hodl();

    void setRootPubKeyWithSignature(const QString & key, const QString & message, const QString & signature);
    // Hodl object changed it's state, need to refresh
    void updateHodlState();
    void reportMessage(const QString & title, const QString & message);
    void hideWaitingStatus();

    Q_INVOKABLE void setColdWalletPublicKey(QString pubkey);

    Q_INVOKABLE void registerAccountForHODL();

    Q_INVOKABLE void moveToClaimPage(QString hash);

    Q_INVOKABLE void requestHodlInfoRefresh(QString hash);

    Q_INVOKABLE void claimMWC(QString hash);

    Q_INVOKABLE void requestSignMessage(QString msg);

signals:
    void sgnSetRootPubKeyWithSignature(QString key, QString hash, QString message, QString signature);

    // Hodl object changed it's state, need to refresh
    void sgnUpdateHodlState();

    void sgnReportMessage(QString title, QString message);

    void sgnHideWaitingStatus();

};

}

#endif //MWC_QT_WALLET_H_HODL_B_H

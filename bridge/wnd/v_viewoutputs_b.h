// Copyright 2021 The MWC Developers
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

//
// Created by Konstantin Bay on 9/23/24.
//

#ifndef MWC_QT_WALLET_VIEWACCOUNTS_H
#define MWC_QT_WALLET_VIEWACCOUNTS_H

#include <QObject>
#include "../../wallet/wallet.h"

namespace bridge {

class ViewOutputs : public QObject {
Q_OBJECT
public:
    explicit ViewOutputs(QObject *parent = nullptr);
    ~ViewOutputs();

    void setSgnViewOutputs( const QVector<wallet::WalletOutput > & outputResult, int64_t total);

    // Switching to the progress window
    Q_INVOKABLE void startScanning(QString hashKey);

    Q_INVOKABLE void backFromOutputsView();
private:
signals:
    void onSgnViewOutputs( QString viewingKey, QVector<QString> outputs, QString totalAmount);
private:
    static QString lastViewingKey;
};

}

#endif //MWC_QT_WALLET_VIEWACCOUNTS_H

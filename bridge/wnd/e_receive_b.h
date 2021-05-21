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

#ifndef MWC_QT_WALLET_E_RECEIVE_B_H
#define MWC_QT_WALLET_E_RECEIVE_B_H

#include <QObject>

namespace bridge {

class Receive : public QObject {
Q_OBJECT
public:
    explicit Receive(QObject * parent = nullptr);
    ~Receive();

    void hideProgress();
    void onTransactionActionIsFinished( bool success, QString message );

    // Validate, ask for continue and then sign transaction.
    // sgnTransactionActionIsFinished will return some feedback if there are any.
    Q_INVOKABLE void signTransaction(QString uriFileName, QString uriDecodedFileName);

    // Sign slatepack transaction. The primary data is a slatepack.  slateJson can be calculated, passed because we
    // already have this info after verification.
    Q_INVOKABLE void signSlatepackTransaction(QString slatepack, QString slateJson, QString slateSenderAddress);

    Q_INVOKABLE void cancelReceive();

    // Files transaction page, continue with a file
    Q_INVOKABLE void receiveFile(QString uriFileName, QString uriDecodedFileName, QString description );
    // Files transaction page, continue with a Slatepack
    Q_INVOKABLE void receiveSlatepack(QString slatepack, QString description);

signals:
    // respond from signTransaction
    void sgnTransactionActionIsFinished( bool success, QString message );

    void sgnHideProgress();
};

}

#endif //MWC_QT_WALLET_E_RECEIVE_B_H

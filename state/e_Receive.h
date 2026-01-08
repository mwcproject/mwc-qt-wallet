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

#ifndef MWC_QT_WALLET_RECEIVE_H
#define MWC_QT_WALLET_RECEIVE_H

#include "state.h"
#include "../wallet/wallet.h"

class QtAndroidService;

namespace state {

const QString RECEIVE_CALLER_ID = "Receive";

class Receive : public QObject, public State {
    Q_OBJECT
public:
    Receive( StateContext * context );
    virtual ~Receive() override;

    void signSlatepackTransaction(QString slatepack, QString slateJson, QString slateSenderAddress);

public:
    void ftBack();
    void receiveSlatepack(QString slatepack, QString description);

    bool needResultTxFileName();

    QString getResultTxPath();

    virtual void updateResultTxPath(QString path);
    virtual bool isNodeHealthy() const;
protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override;
    virtual QString getHelpDocName() override;

    bool isActive() const;

private:
    bool signingFile = false; // what is signing now. File or Slatepack.
    bool atInitialPage = true;
};


}

#endif //MWC_QT_WALLET_RECEIVE_H

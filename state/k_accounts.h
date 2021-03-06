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

#ifndef ACCOUNTSS_H
#define ACCOUNTSS_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QVector>

namespace state {


class Accounts : public QObject, public State
{
    Q_OBJECT
public:
    Accounts(StateContext * context);
    virtual ~Accounts() override;

    void doTransferFunds();

    // ui caller must be at waiting state
    void deleteAccount( QString accountName );
protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override {return false;}
    virtual QString getHelpDocName() override {return "accounts.html";}
private slots:
    void onLoginResult(bool ok);

    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

private:
    virtual void timerEvent(QTimerEvent *event) override;

    bool isNodeHealthy() const {return nodeIsHealthy;}
private:
    bool nodeIsHealthy = false;

    bool lastNodeIsHealty = true;

    int64_t startingTime = 0;
};

}

#endif // ACCOUNTSS_H

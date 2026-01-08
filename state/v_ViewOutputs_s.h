// Copyright 2024 The MWC Developers
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
// Created by Konstantin Bay on 9/22/24.
//

#ifndef MWC_QT_WALLET_ST_VIEWACCOUNTS_H
#define MWC_QT_WALLET_ST_VIEWACCOUNTS_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace state {

class ViewOutputs : public QObject, public State {
public:
    ViewOutputs(StateContext * context);
    virtual ~ViewOutputs() override;

    void startScanning(QString hashKey);
    void backFromOutputsView();

    void generateOwnershipProofStart();
    void generate_proof(QString message, bool viewingKey, bool torAddress, bool mqsAddress);

    void validateOwnershipProofStart();
    void validate_proof(QString proof);

    void back();

protected:
    virtual NextStateRespond execute() override;

    // State can block the stare change. Wallet config is the first usage.
    virtual bool canExitState(STATE nextWindowState) override;
    virtual void exitingState() override;
    virtual bool mobileBack() override;

    virtual QString getHelpDocName() override {return helpDocName;}
private slots:
    void onScanProgress( QString responseId, QJsonObject statusMessage );
    void onScanRewindHash( QString responseId, wallet::ViewWallet walletOutputs, QString error  );
private:
    QString helpDocName = "viewing_key_start.html";
    QString scanRespondId;
};

} // state

#endif //MWC_QT_WALLET_ST_VIEWACCOUNTS_H

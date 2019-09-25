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

#ifndef WALLETCONFIG_H
#define WALLETCONFIG_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace state {

class WalletConfig : public State
{
public:
    WalletConfig( StateContext * context);
    virtual ~WalletConfig() override;

    wallet::WalletConfig    getWalletConfig() const;
    wallet::WalletConfig    getDefaultWalletConfig() const;

    // return true if mwc713 will be restarted. UI suppose to finish asap
    bool setWalletConfig(const wallet::WalletConfig & config, bool guiWalletRestartExpected);

    core::SendCoinsParams   getSendCoinsParams() const;
    // account refresh will be requested...
    void setSendCoinsParams(const core::SendCoinsParams & params);

    double getGuiScale() const;
    double getInitGuiScale() const;
    void updateGuiScale(double scale);

    void restartMwcQtWallet();

    bool getWalletLogsEnabled();
    void updateWalletLogsEnabled(bool enabled, bool needCleanupLogs);
protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "wallet_configuration.html";}
};

}

#endif // WALLETCONFIG_H

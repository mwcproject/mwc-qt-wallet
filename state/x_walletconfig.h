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
    bool setWalletConfig(const wallet::WalletConfig & config);

    core::SendCoinsParams   getSendCoinsParams() const;
    // account refresh will be requested...
    void setSendCoinsParams(const core::SendCoinsParams & params);

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "wallet_configuration.html";}
};

}

#endif // WALLETCONFIG_H

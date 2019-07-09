#ifndef WALLETCONFIG_H
#define WALLETCONFIG_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace state {

class WalletConfig : public State
{
public:
    WalletConfig(const StateContext & context);
    virtual ~WalletConfig() override;

    wallet::WalletConfig    getWalletConfig() const;
    // return true if mwc713 will be restarted. UI suppose to finish asap
    bool setWalletConfig(const wallet::WalletConfig & config);

    core::SendCoinsParams   getSendCoinsParams() const;
    // account refresh will be requested...
    void setSendCoinsParams(const core::SendCoinsParams & params);


protected:
    virtual NextStateRespond execute() override;
};

}

#endif // WALLETCONFIG_H

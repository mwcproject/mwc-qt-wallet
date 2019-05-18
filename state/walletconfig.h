#ifndef WALLETCONFIG_H
#define WALLETCONFIG_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {

class WalletConfig : public State
{
public:
    WalletConfig(const StateContext & context);
    virtual ~WalletConfig() override;

    wallet::WalletConfig getWalletConfig();
    QPair<bool, QString> setWalletConfig(const wallet::WalletConfig & config);

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // WALLETCONFIG_H

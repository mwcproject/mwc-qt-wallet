#ifndef NEWWALLET_H
#define NEWWALLET_H

#include "state.h"

namespace state {


class NewWallet : public State
{
public:
    NewWallet(StateContext * context);
    virtual ~NewWallet() override;

    enum NEW_WALLET_CHOICE { CREATE_NEW, CREATE_WITH_SEED };
    void submitCreateChoice(NEW_WALLET_CHOICE newWalletChoice);

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // NEWWALLET_H

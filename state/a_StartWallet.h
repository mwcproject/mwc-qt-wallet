#ifndef GUI_WALLET_STARTWALLET_H
#define GUI_WALLET_STARTWALLET_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../windows/a_waitingwnd.h"



namespace state {

// Init the wallet. Then check how it is started. If it needs to have password or something
class StartWallet : public State {
public:
    StartWallet(StateContext * context);
    virtual ~StartWallet() override;

protected:
    virtual NextStateRespond execute() override;


private:
};

}

#endif //GUI_WALLET_STARTWALLET_H

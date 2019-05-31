#ifndef GUI_WALLET_STARTWALLET_H
#define GUI_WALLET_STARTWALLET_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {

// Init the wallet. Then check how it is started. If it needs to have password or something
class StartWallet : public QObject, public State {
    Q_OBJECT
public:
    StartWallet(const StateContext & context);
    virtual ~StartWallet() override;

protected:
    virtual NextStateRespond execute() override;

private slots:
    void onInitWalletStatus(wallet::Wallet::InitWalletStatus status);

private:
    QMetaObject::Connection slotConn;
};

}

#endif //GUI_WALLET_STARTWALLET_H

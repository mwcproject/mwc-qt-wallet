#ifndef GUI_WALLET_STARTWALLET_H
#define GUI_WALLET_STARTWALLET_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../windows/a_waitingwnd.h"



namespace state {

// Init the wallet. Then check how it is started. If it needs to have password or something
class StartWallet : public QObject, public State, public wnd::WaitingWndState {
    Q_OBJECT
public:
    StartWallet(StateContext * context);
    virtual ~StartWallet() override;

    virtual void deleteWaitingWnd(wnd::WaitingWnd * w) override { if (w==wnd) {wnd = nullptr;} }

protected:
    virtual NextStateRespond execute() override;

private slots:
    void onInitWalletStatus(wallet::InitWalletStatus status);

private:
    wnd::WaitingWnd * wnd = nullptr;
};

}

#endif //GUI_WALLET_STARTWALLET_H

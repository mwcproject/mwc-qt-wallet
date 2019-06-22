#ifndef MWC_QT_WALLET_SENDONLINEOFFLINE_H
#define MWC_QT_WALLET_SENDONLINEOFFLINE_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
class SendOnlineOffline;
}

namespace state {

class SendOnlineOffline : public QObject, public State {
    Q_OBJECT
public:
    SendOnlineOffline(const StateContext & context);
    virtual ~SendOnlineOffline() override;

    void SendOnlineChosen();
    void SendOfflineChosen();

    void destroyWnd() {wnd = nullptr;}
protected:
    virtual NextStateRespond execute() override;

    void updateBalanceAndContinue();

private slots:
    void onWalletBalanceUpdated();

private:
    wnd::SendOnlineOffline * wnd = nullptr;
    QMetaObject::Connection slotConn;
    STATE nextStep = STATE::NONE;
};

}

#endif // MWC_QT_WALLET_SENDONLINEOFFLINE_H
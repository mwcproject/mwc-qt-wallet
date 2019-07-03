#ifndef MWC_QT_WALLET_ACCOUNTTRANSFER_H
#define MWC_QT_WALLET_ACCOUNTTRANSFER_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace wnd {
class AccountTransfer;
}


namespace state {

class AccountTransfer : public QObject, public State {
Q_OBJECT
public:
    AccountTransfer(const StateContext & context);
    virtual ~AccountTransfer() override;

    void wndDeleted() { wnd = nullptr; }

    // get balance for current account
    QVector<wallet::AccountInfo> getWalletBalance();

    // nanoCoins < 0 - all funds
    void transferFunds(const wallet::AccountInfo & accountFrom,
                       const wallet::AccountInfo & accountTo,
                       int64_t nanoCoins );

    void goBack();

    core::SendCoinsParams getSendCoinsParams();
    void updateSendCoinsParams(const core::SendCoinsParams &params);

protected:
    virtual NextStateRespond execute() override;

private slots:
    // set receive account name results
    void onSetReceiveAccount( bool ok, QString AccountOrMessage );
    void onSend( bool success, QStringList errors );
    void onSlateSend( QString slate, QString mwc, QString sendAddr );
    void onSlateFinalized( QString slate );
    void onWalletBalanceUpdated();

private:
    wnd::AccountTransfer * wnd = nullptr;
    int transferState = -1;

    // Single transfer context
    QString myAddress;
    wallet::AccountInfo trAccountFrom;
    wallet::AccountInfo trAccountTo;
    int64_t trNanoCoins = 0;
    QString trSlate;
};

}

#endif //MWC_QT_WALLET_ACCOUNTTRANSFER_H

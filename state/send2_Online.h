#ifndef MWC_QT_WALLET_SENDONLINE_H
#define MWC_QT_WALLET_SENDONLINE_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace wnd {
    class SendOnline;
}

namespace state {

class SendOnline : public QObject, public State {
    Q_OBJECT
public:
    SendOnline( StateContext * context );
    virtual ~SendOnline() override;

    void deleteWnd(wnd::SendOnline * w) { if (w==wnd) wnd = nullptr;}

    QVector<wallet::AccountInfo> getWalletBalance();
    QString getCurrentAccountName();

    core::SendCoinsParams getSendCoinsParams();
    void updateSendCoinsParams( const core::SendCoinsParams  & params );

    // Request for MWC to send
    void sendMwc( const wallet::AccountInfo & account, QString address, int64_t mwcNano, QString message );

protected:
    virtual NextStateRespond execute() override;

private slots:
    void sendRespond( bool success, QStringList errors );

private:
    wnd::SendOnline * wnd = nullptr; // not own, just a pointer
};


}


#endif //MWC_QT_WALLET_SENDONLINE_H

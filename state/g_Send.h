#ifndef MWC_QT_WALLET_SEND_H
#define MWC_QT_WALLET_SEND_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace wnd {
class SendStarting;
class SendOnline;
class SendOffline;
}

namespace state {

class Send  : public QObject, public State {
    Q_OBJECT
public:
    Send( StateContext * context);
    virtual ~Send() override;

    void destroyOnlineOfflineWnd(wnd::SendStarting * w) { if (w==onlineOfflineWnd) onlineOfflineWnd = nullptr;}
    void destroyOnlineWnd(wnd::SendOnline * w) { if (w==onlineWnd) onlineWnd = nullptr;}
    void destroyOfflineWnd(wnd::SendOffline * w) { if (w==offlineWnd) offlineWnd = nullptr;}

    core::SendCoinsParams getSendCoinsParams();
    void updateSendCoinsParams( const core::SendCoinsParams  & params );

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

    // onlineOffline => Next step
    void processSendRequest( bool isOnline, const wallet::AccountInfo & selectedAccount, int64_t amount );

    // Request for MWC to send
    void sendMwcOnline( const wallet::AccountInfo & account, QString address, int64_t mwcNano, QString message );

    void sendMwcOffline(  const wallet::AccountInfo & account, int64_t amount, QString message, QString fileName );

protected:
    virtual NextStateRespond execute() override;


private slots:
    void onWalletBalanceUpdated();
    void sendRespond( bool success, QStringList errors );
    void respSendFile( bool success, QStringList errors, QString fileName );

private:
    void switchToStartingWindow();

private:
    wnd::SendStarting  * onlineOfflineWnd = nullptr;
    wnd::SendOnline         * onlineWnd = nullptr;
    wnd::SendOffline  * offlineWnd = nullptr;


};

}

#endif
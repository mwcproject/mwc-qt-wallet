#ifndef MWC_QT_WALLET_SEND3_OFFLINE_H
#define MWC_QT_WALLET_SEND3_OFFLINE_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace wnd {
class SendOfflineFiles;
class SendOfflineSettings;
}

namespace state {

class SendOffline : public QObject, public State {
    Q_OBJECT
public:
    SendOffline(const StateContext & context);
    virtual ~SendOffline() override;

    QVector<wallet::AccountInfo> getWalletBalance();
    QString getCurrentAccountName();

    core::SendCoinsParams getSendCoinsParams();
    void updateSendCoinsParams( const core::SendCoinsParams  & params );

    void prepareSendMwcOffline( const wallet::AccountInfo & account, QString message );

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

    void sendToFile(int64_t nanoCoins, QString fileName);
    //void signTransaction( QString fileName );
    void publishTransaction( QString fileName );

    void deletedSendOfflineFiles() {filesWnd = nullptr;}
    void deletedSendOfflineSettings() {settingsWnd = nullptr;}
protected:
    virtual NextStateRespond execute() override;

    void respSendFile( bool success, QStringList errors, QString fileName );
    //void respReceiveFile( bool success, QStringList errors, QString inFileName );
    void respFinalizeFile( bool success, QStringList errors, QString fileName );

private:
    wnd::SendOfflineSettings * settingsWnd = nullptr;
    wnd::SendOfflineFiles * filesWnd = nullptr;
    QString message;
};

}



#endif //MWC_QT_WALLET_SEND3_OFFLINE_H

#ifndef MWC_QT_WALLET_RECIEVE_H
#define MWC_QT_WALLET_RECIEVE_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
class Recieve;
}

namespace state {

class Recieve : public QObject, public State {
    Q_OBJECT
public:
    Recieve( StateContext * context );
    virtual ~Recieve() override;

    QString  getReceiveAccount();
    void  setReceiveAccount(QString accountName );
    QVector<wallet::AccountInfo>  getWalletBalance();

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

    void signTransaction( QString fileName );

    void deletedWnd(wnd::Recieve * w) { if(w==wnd) wnd = nullptr;}
protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "receive.html";}

    void respReceiveFile( bool success, QStringList errors, QString inFileName );

private slots:
    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);
    void onMwcAddressWithIndex(QString mwcAddress, int idx);

private:
    wnd::Recieve * wnd = nullptr;
};


}

#endif //MWC_QT_WALLET_RECIEVE_H

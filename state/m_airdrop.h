#ifndef AIRDROP_H
#define AIRDROP_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QObject>
#include <QMap>

class QNetworkAccessManager;
class QNetworkReply;

namespace wnd {
    class Airdrop;
    class AirdropForBTC;
}

namespace state {

struct AirdropRequests {
    QString btcAddress;
    QString challendge;
    QString signature;
    int64_t timestamp; // ms since epoch

    void setData(const QString & btcAddress,
                 const QString & challendge,
                 const QString & signature);

    void saveData(QDataStream & out) const;
    bool loadData(QDataStream & in);
};

struct AirDropStatus {
    bool waiting = false;
    bool status = false;
    QString message = "Status Request is in progress";
};

class Airdrop : public QObject, public State
{
    Q_OBJECT
public:
    Airdrop( StateContext * context);
    virtual ~Airdrop() override;

    void deleteAirdropForBtcWnd(wnd::AirdropForBTC * w) { if (w==airdropForBtcWnd) airdropForBtcWnd = nullptr;}
    void deleteAirdropWnd(wnd::Airdrop * w) { if (w==airdropWnd) airdropWnd = nullptr;}

    const AirDropStatus & getAirDropStatus() const {return airDropStatus;}

    bool hasAirdropRequests() const {return airdropRequests.size()>0;}

    void startClaimingProcess( const QString & btcAddress, const QString & passwordAirdrop );
    void requestClaimMWC( const QString & btcAddress, const QString & challendge, const QString & signature, const QString & identifier );
    // Get current status for that
    void refreshAirdropStatusInfo();

    QVector<int> getColumnsWidhts();
    void updateColumnsWidhts(QVector<int> widths);

    void backToMainAirDropPage();
protected:
    enum HTTP_CALL {GET, POST};
    void sendRequest(HTTP_CALL call, const QString & url,
                const QVector<QString> & urlParams,
                const QByteArray & body, //
                const QString & tag, const QString & param1="",const QString & param2="",
                const QString & param3="", const QString & param4=""); // Respond will be back at replyFinished.  replyFinished will process it accordingly

    // Respond with error to UI. UI expected to stop waiting
    void reportMessageToUI( QString title, QString message );

    // Update the status into UI
    void updateAirdropRequestsStatus( int idx, int total, AirdropRequests req, QString status, int64_t mwcNano);

    // Initiate a request for btc claim. Next indexes will be called automatically
    void requestStatusFor(int idx);

    virtual NextStateRespond execute() override;

    virtual void exitingState() override;

    virtual QString getHelpDocName() override {return "Airdrop.html";}

private slots:
    void replyFinished(QNetworkReply* reply);

    void onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor);

    void onReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
private:
    QVector<AirdropRequests> airdropRequests;
    QNetworkAccessManager *nwManager;

    AirDropStatus airDropStatus;

    wnd::Airdrop * airdropWnd = nullptr;
    wnd::AirdropForBTC * airdropForBtcWnd = nullptr;

    int requestCounter = 0;
    QMap<QString, QString> claimRequests; // Key: slate file; value: btcAddress
};

}


#endif // AIRDROP_H

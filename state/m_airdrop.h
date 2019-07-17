#ifndef AIRDROP_H
#define AIRDROP_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QObject>

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
    QString mwcMqAddress;
    int64_t timestamp;

    void setData(const QString & btcAddress,
                 const QString & challendge,
                 const QString & signature,
                 const QString & mwcMqAddress );

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
    Airdrop(const StateContext & context);
    virtual ~Airdrop() override;

    void deleteAirdropForBtcWnd() {airdropForBtcWnd = nullptr;}
    void deleteAirdropWnd() {airdropWnd = nullptr;}

    const AirDropStatus & getAirDropStatus() const {return airDropStatus;}

    bool hasAirdropRequests() const {return airdropRequests.size()>0;}

    void requestGetChallenge( QString btcAddress );
    void requestClaimMWC( QString btcAddress, QString challendge, QString signature );
    // Get current status for that
    void refreshAirdropStatusInfo();

    QVector<int> getColumnsWidhts();
    void updateColumnsWidhts(QVector<int> widths);

    void backToMainAirDropPage();
protected:
    void sendRequest(const QString & url,
                const QVector<QString> & body,
                const QString & tag, const QString & param1="",const QString & param2="",
                const QString & param3="", const QString & param4=""); // Respond will be back at replyFinished.  replyFinished will process it accordingly

    // Respond with error to UI. UI expected to stop waiting
    void reportMessageToUI( QString title, QString message );

    // Update the status into UI
    void updateAirdropRequestsStatus( int idx, int total, AirdropRequests req, QString status, int64_t mwcNano);

    // Initiate a request for btc claim. Next indexes will be called automatically
    void requestStatusFor(int idx);

    virtual NextStateRespond execute() override;

private slots:
    void replyFinished(QNetworkReply* reply);

private:
    QVector<AirdropRequests> airdropRequests;
    QNetworkAccessManager *nwManager;

    AirDropStatus airDropStatus;

    wnd::Airdrop * airdropWnd = nullptr;
    wnd::AirdropForBTC * airdropForBtcWnd = nullptr;
};

}


#endif // AIRDROP_H

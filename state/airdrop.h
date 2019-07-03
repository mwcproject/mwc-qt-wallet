#ifndef AIRDROP_H
#define AIRDROP_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {

struct AirdropChallendge {
    QString btcAddress;
    bool    ok = false;
    QString challendge;
    QString errMessage;

    void initOk( QString btcAddress, QString challendge );
    void initFail(QString btcAddress, QString errMessage);
    void reset();
    bool isEmpty();
};

struct AirdropRequestsStatus {
    QString btcAddress;
    QString challendge;
    QString signature;
    bool    ok = false;
    QString status; // ok status
    QString errMessage;
    int64_t    nanoCoins = 0; // amount that was requested

    void initOk(QString btcAddress, QString challendge, QString signature,
            QString status, int64_t nanoCoins );

    void initFail( QString btcAddress, QString challendge, QString signature,
                   QString errMessage);
};

class Airdrop : public State
{
public:
    Airdrop(const StateContext & context);
    virtual ~Airdrop() override;

    QPair<bool, QString> claimBtcAddress( QString address );

    AirdropChallendge getLastAirdropChallendge() const {return lastChallendge;}

    QPair<bool, QString> requestMWC( QString btcAddress, QString challendge, QString signature );

    // Get current status for that
    QVector<AirdropRequestsStatus> getAirdropStatusInfo();

    QPair<bool, QString> refreshStatus();

    QVector<int> getColumnsWidhts();
    void updateColumnsWidhts(QVector<int> widths);

    void backToMainAirDropPage();
protected:

    virtual NextStateRespond execute() override;

private:
    AirdropChallendge lastChallendge;
    QVector<AirdropRequestsStatus> transactionsStatus;


};

}


#endif // AIRDROP_H

#include "airdrop.h"

#include "../wallet/wallet.h"
#include "../windows/airdrop_w.h"
#include "../windows/airdropforbtc_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

    void AirdropChallendge::initOk( QString _btcAddress, QString _challendge ) {
        btcAddress = _btcAddress;
        ok = true;
        challendge = _challendge;
        errMessage = "";
    }
    void AirdropChallendge::initFail(QString _btcAddress, QString _errMessage) {
        btcAddress = _btcAddress;
        ok = false;
        challendge = "";
        errMessage = _errMessage;
    }

    void AirdropChallendge::reset() {
        btcAddress = "";
        ok = false;
        challendge = "";
        errMessage = "";
    }

    bool AirdropChallendge::isEmpty() {
        return btcAddress.length() == 0;
    }


    void AirdropRequestsStatus::initOk(QString _btcAddress, QString _challendge, QString _signature,
                                       QString _status, int64_t _nanoCoins ) {
        btcAddress = _btcAddress;
        challendge = _challendge;
        signature  = _signature;
        ok = true;
        status = _status;
        errMessage = "";
        nanoCoins = _nanoCoins;
    }

    void AirdropRequestsStatus::initFail( QString _btcAddress, QString _challendge, QString _signature,
                                          QString _errMessage) {
        btcAddress = _btcAddress;
        challendge = _challendge;
        signature  = _signature;
        ok = false;
        status = "";
        errMessage = _errMessage;
        nanoCoins = 0;
    }


Airdrop::Airdrop(const StateContext & context ) :
        State(context, STATE::AIRDRDOP_MAIN)
{
}

Airdrop::~Airdrop() {}

NextStateRespond Airdrop::execute() {
    if (context.appContext->getActiveWndState() != STATE::AIRDRDOP_MAIN)
        return NextStateRespond(NextStateRespond::RESULT::DONE);


    if (lastChallendge.isEmpty()) {
        context.wndManager->switchToWindow(
                    new wnd::Airdrop( context.wndManager->getInWndParent(), this ) );
    }
    else {
        context.wndManager->switchToWindow(
            new wnd::AirdropForBTC( context.wndManager->getInWndParent(), this ) );
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


QPair<bool, QString> Airdrop::claimBtcAddress( QString address ) {
    // From here we suppose to call Rest API and get the info

    lastChallendge.initOk(address, "MWX000CHALLENGE000");

    context.stateMachine->executeFrom(STATE::AIRDRDOP_MAIN);

    return QPair<bool, QString>(true,"");
}

QPair<bool, QString> Airdrop::requestMWC( QString btcAddress, QString challendge, QString signature ) {
    // We chould request the transfer. Wait for respond. Get it back

    AirdropRequestsStatus status;
    status.initOk(btcAddress,challendge,signature, "Waiting", 27000000000l );
    transactionsStatus.push_back(status);

    return QPair<bool, QString>(true,"");
}

QVector<AirdropRequestsStatus> Airdrop::getAirdropStatusInfo() {
    return transactionsStatus;
}

QPair<bool, QString> Airdrop::refreshStatus() {
    // Here we shoudl call API to refresh the statuses

    return QPair<bool, QString>(true,"");
}

void Airdrop::backToMainAirDropPage() {
    lastChallendge.reset();
    context.stateMachine->executeFrom(STATE::AIRDRDOP_MAIN);
}

QVector<int> Airdrop::getColumnsWidhts() {
    return context.appContext->getIntVectorFor("AirdropTblWidth");
}

void Airdrop::updateColumnsWidhts(QVector<int> widths) {
    context.appContext->updateIntVectorFor("AirdropTblWidth", widths);
}



}

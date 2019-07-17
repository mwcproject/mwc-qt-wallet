#include "m_airdrop.h"

#include "../wallet/wallet.h"
#include "windows/m_airdrop_w.h"
#include "windows/m_airdropforbtc_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../state/statemachine.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <control/messagebox.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include "../util/Log.h"

namespace state {

static const QString TAG_CLAIMS_AVAIL  = "claimsAvailable";
static const QString TAG_GET_CHALLENGE = "getChallenge";
static const QString TAG_CLAIM_MWC     = "claimMwc";
static const QString TAG_STATUS_PREF   = "Status_";

void AirdropRequests::setData(const QString & _btcAddress,
             const QString & _challendge,
             const QString & _signature,
             const QString & _mwcMqAddress) {
    btcAddress = _btcAddress;
    challendge = _challendge;
    signature  = _signature;
    mwcMqAddress = _mwcMqAddress;
    timestamp = QDateTime::currentMSecsSinceEpoch();
}

void AirdropRequests::saveData(QDataStream & out) const {
    out << int(0x89F15);
    out << btcAddress;
    out << challendge;
    out << signature;
    out << mwcMqAddress;
    out << timestamp;
}

bool AirdropRequests::loadData(QDataStream & in) {
    int id = 0;
    in >> id;

    if ( id!=0x89F15 )
        return false;

    in >> btcAddress;
    in >> challendge;
    in >> signature;
    in >> mwcMqAddress;
    in >> timestamp;
    return true;
}


Airdrop::Airdrop(const StateContext & context ) :
        State(context, STATE::AIRDRDOP_MAIN)
{
    nwManager = new QNetworkAccessManager(this);

    connect( nwManager, &QNetworkAccessManager::finished, this, &Airdrop::replyFinished );

    // request AirDrop Status in background
    airDropStatus.waiting = true;
    sendRequest( "/v1/claimsAvailable", {}, TAG_CLAIMS_AVAIL);

    airdropRequests = context.appContext->loadAirdropRequests();
}

Airdrop::~Airdrop() {}

NextStateRespond Airdrop::execute() {
    if (context.appContext->getActiveWndState() != STATE::AIRDRDOP_MAIN)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    airdropWnd = (wnd::Airdrop *) context.wndManager->switchToWindowEx(
                    new wnd::Airdrop( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


void Airdrop::requestGetChallenge( QString address ) {
    // From here we suppose to call Rest API and get the info

    QString mqAddress = context.wallet->getLastKnownMwcBoxAddress();

    if (mqAddress.isEmpty() || !context.wallet->getListeningStatus().first) {
        reportMessageToUI("MWC MQ Connection", "MWC MQ listener is offline. Please verify your network connection and check if MWC MQ listener running." );
        return;
    }

    sendRequest("/v1/getChallenge" ,
            {"btcaddress", address, "mwcaddress", mqAddress},
                TAG_GET_CHALLENGE, address );

}

void Airdrop::requestClaimMWC( QString btcAddress, QString challendge, QString signature ) {

    QString mqAddress = context.wallet->getLastKnownMwcBoxAddress();

    if (mqAddress.isEmpty() || !context.wallet->getListeningStatus().first) {
        reportMessageToUI("MWC MQ Connection", "MWC MQ listener is offline. Please verify your network connection and check if MWC MQ listener running." );
        return;
    }

    sendRequest("/v1/claimMWC" ,
                {"btcaddress", btcAddress, "challenge", challendge, "signature", signature, "mwcaddress", mqAddress },
                TAG_CLAIM_MWC, btcAddress, challendge, signature, mqAddress);
}


void Airdrop::refreshAirdropStatusInfo() {
    if (airdropRequests.size()==0)
        return; // no refresh to do

    requestStatusFor(0);
}

void Airdrop::requestStatusFor(int idx) {
    sendRequest("/v1/checkStatus" ,
                {"btcaddress", airdropRequests[idx].btcAddress },
                TAG_STATUS_PREF+ QString::number(idx) );
}

void Airdrop::backToMainAirDropPage() {
    airdropWnd = (wnd::Airdrop *) context.wndManager->switchToWindowEx(
            new wnd::Airdrop( context.wndManager->getInWndParent(), this ) );
}

QVector<int> Airdrop::getColumnsWidhts() {
    return context.appContext->getIntVectorFor("AirdropTblWidth");
}

void Airdrop::updateColumnsWidhts(QVector<int> widths) {
    context.appContext->updateIntVectorFor("AirdropTblWidth", widths);
}


void Airdrop::sendRequest(const QString & api,
                          const QVector<QString> & params,
                          const QString & tag, const QString & param1, const QString & param2,
                          const QString & param3, const QString & param4) {

    QString url = core::Config::getAirdropUrl() + api;

    qDebug() << "Sending request: " << url << ", params: " << params << "  tag:" << tag;

    QUrl requestUrl(url);

    // enrich with params
    Q_ASSERT( params.size()%2==0 );
    QUrlQuery query;
    for (int t=1; t<params.size(); t+=2) {
        query.addQueryItem(params[t-1], params[t]);
    }
    requestUrl.setQuery(query);

    QNetworkRequest request;

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(config);

    qDebug() << "Processing: GET " << requestUrl.toString();
    logger::logInfo("Airdrop", "Requesting: " + requestUrl.toString());
    request.setUrl( requestUrl );
    request.setHeader(QNetworkRequest::ServerHeader, "application/json");


    QNetworkReply *reply =  nwManager->get(request);
    reply->setProperty("tag", QVariant(tag));
    reply->setProperty("param1", QVariant(param1));
    reply->setProperty("param2", QVariant(param2));
    reply->setProperty("param3", QVariant(param3));
    reply->setProperty("param4", QVariant(param4));
    // Respond will be send back async
}

void Airdrop::replyFinished(QNetworkReply* reply) {
    QNetworkReply::NetworkError errCode = reply->error();
    QString tag = reply->property("tag").toString();

    qDebug() << "Get back respond with tag: " << tag << "  Error code: " << errCode;

    QJsonObject jsonRespond;
    bool  requestOk = false;
    QString requestErrorMessage;

    if (reply->error() == QNetworkReply::NoError) {
        requestOk = true;

         // read the reply body
        QString strReply (reply->readAll().trimmed());
        qDebug() << "Get back respond. Tag: " << tag << "  Reply " << strReply;
        logger::logInfo("Airdrop", "Success respond for Tag: " + tag + "  Reply " + strReply);

        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(strReply.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            requestOk = false;
            requestErrorMessage = "Unable to parse respond Json at position " + QString::number(error.offset) +
                    "\nJson string: " + strReply;
        }
        jsonRespond = jsonDoc.object();
    }
    else  {
        requestOk = false;
        requestErrorMessage = reply->errorString();
        logger::logInfo("Airdrop", "Fail respond for Tag: " + tag + "  requestErrorMessage: " + requestErrorMessage);
    }
    reply->deleteLater();

    // Done with reply. Now processing the results by tags


    if ( TAG_CLAIMS_AVAIL == tag) {
        airDropStatus.waiting = false;
        if (requestOk) {
            airDropStatus.status = jsonRespond["status"].toBool(false);
            airDropStatus.message = jsonRespond["message"].toString();
        } else {
            airDropStatus.status = false;
            airDropStatus.message = "Unable to request the status info from " + core::Config::getAirdropUrl() +
                                    ".\nGet communication error: " + requestErrorMessage;
        }

        if (airdropWnd) {
            airdropWnd->updateAirDropStatus(airDropStatus);
        }
        return;
    }

    if (!requestOk) {
        reportMessageToUI("Network error", "Unable to communicate with " + core::Config::getAirdropUrl() + ".\nGet communication error: " + requestErrorMessage);
        return;
    }


    if ( TAG_GET_CHALLENGE == tag ) {
            bool success = jsonRespond["success"].toBool(false);
            if ( success ) {
                QString address = reply->property("param1").toString();

                QString challenge = jsonRespond["challenge"].toString();
                // Switch to the claim window...
                airdropForBtcWnd = (wnd::AirdropForBTC *) context.wndManager->switchToWindowEx(
                        new wnd::AirdropForBTC( context.wndManager->getInWndParent(), this, address, challenge ) );
            }
            else {
                // error status
                int errCode = jsonRespond["error_code"].toInt(-1);
                QString errMessage = jsonRespond["error_message"].toString();

                reportMessageToUI("Claim request failed", "Unable to start claim process.\n" + (errCode>0? "Error Code: " + QString::number(errCode)+"\n" : "") +  errMessage );
            }
    }

    if (TAG_CLAIM_MWC == tag) {
        bool success = jsonRespond["success"].toBool(false);
        QString errMessage = jsonRespond["error_message"].toString();
        int errCode = jsonRespond["error_code"].toInt(-1);

        QString btcAddress = reply->property("param1").toString();
        QString challendge = reply->property("param2").toString();
        QString signature = reply->property("param3").toString();
        QString mwcAddress = reply->property("param4").toString();

        if (success) {
            // add to the list of claimed...
            bool found = false;
            for (auto &req : airdropRequests) {
                if (req.btcAddress == btcAddress) {
                    req.setData(btcAddress, challendge, signature, mwcAddress);
                    found = true;
                }
            }

            if (!found) {
                AirdropRequests newReq;
                newReq.setData( btcAddress, challendge, signature, mwcAddress );

                airdropRequests.push_back(newReq);
                context.appContext->saveAirdropRequests( airdropRequests );
            }

            reportMessageToUI("Your MWC claim succeeded", "Your clain for address " + btcAddress + "was sucessfully processed" + (errMessage.isEmpty() ? "" : "\n" + errMessage) );

            backToMainAirDropPage();
        }
        else {
            reportMessageToUI("Claim request failed", "Unable to process your claim.\n" + (errCode>0? "Error Code: " + QString::number(errCode)+"\n" : "") +  errMessage );
        }
    }

    if ( tag.startsWith(TAG_STATUS_PREF)) {
        // Checking the index
        int idx = tag.mid( TAG_STATUS_PREF.length() ).toInt();

        int errCode = jsonRespond["code"].toInt(-1);
        int64_t amount = jsonRespond["amount_sent"].toString("-1").toLongLong();
        QString status = jsonRespond["status"].toString();
        QString message = jsonRespond["response"].toString();

        if (status.isEmpty())
            status = message;

        Q_ASSERT( idx>=0 && idx<airdropRequests.size() );
        if ( idx>=0 && idx<airdropRequests.size() && airdropWnd ) {
            // updating and request next if needed
            if ( airdropWnd ) {
                if (airdropWnd->updateClaimStatus( idx, airdropRequests[idx], status, message, amount, errCode)) {
                    // Request next...
                    if (idx + 1 < airdropRequests.size())
                        requestStatusFor(idx + 1);
                }
            }
        }
    }

}

// Respond with error to UI. UI expected to stop waiting
void Airdrop::reportMessageToUI( QString title, QString message ) {
    if (airdropWnd)
        airdropWnd->reportMessage(title, message);
    else if (airdropForBtcWnd)
        airdropForBtcWnd->reportMessage(title, message);
}




}

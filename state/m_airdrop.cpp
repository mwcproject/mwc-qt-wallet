// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "m_airdrop.h"

#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../state/statemachine.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include "../util/Log.h"
#include <QFile>
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/m_airdrop_b.h"
#include <QDataStream>

namespace state {

static const QString TAG_CLAIMS_AVAIL  = "airdrop_claimsAvailable";
static const QString TAG_AMOUNT        = "airdrop_getAmount";
static const QString TAG_GET_CHALLENGE = "airdrop_getChallenge";
static const QString TAG_CLAIM_MWC     = "airdrop_claimMwc";
static const QString TAG_RESPONCE_SLATE = "airdrop_submitResponseSlate";
static const QString TAG_STATUS_PREF   = "airdrop_Status_";

void AirdropRequests::setData(const QString & _btcAddress,
             const QString & _challendge, const QString & _signature) {
    btcAddress = _btcAddress;
    challendge = _challendge;
    signature = _signature;
    timestamp = QDateTime::currentMSecsSinceEpoch();
}

void AirdropRequests::saveData(QDataStream & out) const {
    out << int(0x89F17);
    out << btcAddress;
    out << challendge;
    out << signature;
    out.writeRawData( (char*)&timestamp, sizeof(timestamp));
}

bool AirdropRequests::loadData(QDataStream & in) {
    int id = 0;
    in >> id;

    if ( id!=0x89F17 )
        return false;

    in >> btcAddress;
    in >> challendge;
    in >> signature;
    in.readRawData( (char*) &timestamp, sizeof(timestamp));
    return true;
}


Airdrop::Airdrop(StateContext * context ) :
        State(context, STATE::AIRDRDOP_MAIN)
{
    nwManager = new QNetworkAccessManager(this);

    connect( nwManager, &QNetworkAccessManager::finished, this, &Airdrop::replyFinished, Qt::QueuedConnection );
    connect( context->wallet , &wallet::Wallet::onReceiveFile, this, &Airdrop::onReceiveFile, Qt::QueuedConnection );
    connect( context->wallet , &wallet::Wallet::onGetNextKeyResult, this, &Airdrop::onGetNextKeyResult, Qt::QueuedConnection );
    connect( context->wallet , &wallet::Wallet::onLoginResult, this, &Airdrop::onLoginResult, Qt::QueuedConnection );

    // request AirDrop Status in background
    airDropStatus.waiting = true;

    airdropRequests = context->appContext->loadAirdropRequests();
}

Airdrop::~Airdrop() {}

NextStateRespond Airdrop::execute() {
    if (context->appContext->getActiveWndState() != STATE::AIRDRDOP_MAIN)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (bridge::getBridgeManager()->getAirdrop().isEmpty()) {
        core::getWndManager()->pageAirdrop();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Airdrop::onLoginResult(bool ok) {
    Q_UNUSED(ok)

    airDropStatus.waiting = true;
    airDropUrl = (context->wallet->getWalletConfig().getNetwork() == "Mainnet") ?
                config::getAirdropMainNetUrl() : config::getAirdropTestNetUrl();

    sendRequest( HTTP_CALL::GET, "/v1/claimsAvailable", {}, "", TAG_CLAIMS_AVAIL);
}


void Airdrop::startClaimingProcess( const QString & btcAddress, const QString & passwordAirdrop ) {
    // From here we suppose to call Rest API and get the info

    // First, let's get amount.
    sendRequest( HTTP_CALL::GET, "/v1/getAmount" ,
                {"password", passwordAirdrop,  "btcaddress", btcAddress },
                "",
                TAG_AMOUNT, passwordAirdrop, btcAddress );

    // Then get challenge
    //...
}

void Airdrop::requestClaimMWC( const QString & btcAddress, const QString & challendge, const QString & signature, const QString & identifier ) {
    sendRequest( HTTP_CALL::GET, "/v1/claimMWC" ,
                {"btcaddress", btcAddress, "challenge", challendge, "signature", signature },
                "",
                TAG_CLAIM_MWC, btcAddress, challendge, signature, identifier);
}


void Airdrop::refreshAirdropStatusInfo() {
    if (airdropRequests.size()==0)
        return; // no refresh to do

    requestStatusFor(0);
}

void Airdrop::requestStatusFor(int idx) {
    // https://<api_endpoint>/v1/checkStatus?btcaddress=<btcaddress>&challenge=<challenge>
    sendRequest(HTTP_CALL::GET, "/v1/checkStatus" ,
                { "btcaddress", airdropRequests[idx].btcAddress, "challenge", airdropRequests[idx].challendge,
                  "signature" , airdropRequests[idx].signature },
                "",
                TAG_STATUS_PREF+ QString::number(idx) );
}

void Airdrop::backToMainAirDropPage() {
    core::getWndManager()->pageAirdrop();
}

void Airdrop::sendRequest(HTTP_CALL call, const QString & api,
                          const QVector<QString> & params,
                          const QByteArray & body, //
                          const QString & tag, const QString & param1, const QString & param2,
                          const QString & param3, const QString & param4) {

    QString url = airDropUrl + api;

    qDebug() << "Sending request: " << url << ", params: " << params << "  tag:" << tag;

    QUrl requestUrl(url);

    // enrich with params
    Q_ASSERT( params.size()%2==0 );
    QUrlQuery query;
    for (int t=1; t<params.size(); t+=2) {
        query.addQueryItem( util::urlEncode(params[t-1]), util::urlEncode(params[t]));
    }
    // Note: QT encoding has issues, some symbols will be skipped.
    // No encoding needed because we encode params with out code.
    requestUrl.setQuery(query.query(QUrl::PrettyDecoded), QUrl::StrictMode );

    QNetworkRequest request;

    // sslLibraryVersionString neede as a workaroung for a deadlock at defaultConfiguration, qt v5.9
    QSslSocket::sslLibraryVersionString();
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(config);

    qDebug() << "Processing: GET " << requestUrl.toString(QUrl::FullyEncoded);
    logger::logInfo("Airdrop", "Requesting: " + url );
    request.setUrl( requestUrl );
    request.setHeader(QNetworkRequest::ServerHeader, "application/json");

    QNetworkReply *reply = nullptr;
    switch (call) {
        case GET:
            reply =  nwManager->get(request);
            break;
        case POST:
            reply =  nwManager->post(request, body );
            break;
        default:
            Q_ASSERT(false);
    }
    Q_ASSERT(reply);

    if (reply) {
        reply->setProperty("tag", QVariant(tag));
        reply->setProperty("param1", QVariant(param1));
        reply->setProperty("param2", QVariant(param2));
        reply->setProperty("param3", QVariant(param3));
        reply->setProperty("param4", QVariant(param4));
        // Respond will be send back async
    }
}

static QString generateMessage( QString errMessage, int errCode ) {
    QString message;
    if (!errMessage.isEmpty()) {
        message += "\n" + errMessage;
        if (errCode != INT_MAX) {
            message += " (Code: " + QString::number( errCode ) + ")";
        }
    }
    return message;
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
            airDropStatus.message = "Unable to request the status info from " + airDropUrl +
                                    ".\nGet communication error: " + requestErrorMessage;
        }

        for (auto b : bridge::getBridgeManager()->getAirdrop())
            b->updateAirDropStatus(airDropStatus.waiting, airDropStatus.status, airDropStatus.message);

        return;
    }

    if (!requestOk) {
        reportMessageToUI("Network error", "Unable to communicate with " + airDropUrl + ".\nGet communication error: " + requestErrorMessage);
        return;
    }

    if ( TAG_AMOUNT == tag ) {
        bool success = jsonRespond["success"].toBool(false);
        if ( success ) {
            int64_t amount = jsonRespond["amount"].toString("-1").toLongLong();

            QString password = reply->property("param1").toString();
            QString address = reply->property("param2").toString();

            // Preparing for the getChallenge...
            // Expected that walllet is offline
            context->wallet->getNextKey( amount, address, password );
            // see onGetNextKeyResult to continue...
        }
        else {
            // error status
            reportMessageToUI("Claim request failed", "Unable to start claim process." +
                                                      generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) );
        }
        return;
    }

    if ( TAG_GET_CHALLENGE == tag ) {
            bool success = jsonRespond["success"].toBool(false);
            if ( success ) {
                QString address = reply->property("param1").toString();
                QString identifier = reply->property("param2").toString();

                QString challenge = jsonRespond["challenge"].toString();
                // Switch to the claim window...
                core::getWndManager()->pageAirdropForBTC(address, challenge, identifier);
            }
            else {
                // error status
                reportMessageToUI("Claim request failed", "Unable to start claim process." +
                                  generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) );
            }
        return;
    }

    if (TAG_CLAIM_MWC == tag) {
        bool success = jsonRespond["success"].toBool(false);

        QString btcAddress = reply->property("param1").toString();
        QString challendge = reply->property("param2").toString();
        QString signature = reply->property("param3").toString();
        QString identifier = reply->property("param4").toString();

        Q_ASSERT( !btcAddress.isEmpty() && !challendge.isEmpty() && !signature.isEmpty() && !identifier.isEmpty() );

        if ( success && !btcAddress.isEmpty() && !challendge.isEmpty() && !signature.isEmpty() && !identifier.isEmpty() ) {
            QString errMsg;
            while (true) {
                QString slateBase64 = jsonRespond["slate"].toString();
                // Expected to have a base64 encoded slate
                if (slateBase64.isEmpty()) {
                    errMsg = "Not found slate data from AirDrop API respond";
                    break;
                }

                QByteArray slate = QByteArray::fromBase64(slateBase64.toLatin1());

                if (slate.isEmpty()) {
                    errMsg = "Not able to decode slate from AirDrop API respond";
                    break;
                }

                QPair<bool,QString> tmpPath = ioutils::getAppDataPath("tmp");
                if (!tmpPath.first) {
                    errMsg = tmpPath.second;
                }

                QString slateFn = tmpPath.second + "/" + "slate"+QString::number(requestCounter++)+".tx";
                {
                    // Clean up response if exist
                    QString slateRespFn = slateFn + ".response";
                    QFile::remove(slateRespFn);
                }

                claimRequests[slateFn] = btcAddress;

                QFile slateFile(slateFn);
                if (slateFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    slateFile.write(slate);
                }
                else {
                    errMsg = "Not able to save slate at the file " + slateFn;
                    break;

                }
                slateFile.close();

                // Registering the transaction even it is not finished. The reason - possible failure.
                // We better have non complete data then lost one. In case of lost,
                // no tracking status will be possible...
                {
                    bool found = false;
                    for (auto &req : airdropRequests) {
                        if (req.btcAddress == btcAddress) {
                            req.setData(btcAddress, challendge, signature);
                            found = true;
                        }
                    }

                    if (!found) {
                        AirdropRequests newReq;
                        newReq.setData(btcAddress, challendge, signature);
                        airdropRequests.push_back(newReq);
                    }
                    context->appContext->saveAirdropRequests(airdropRequests);
                }

                // Starting the slate processing transaction...
                // wallet expected to be offline...
                context->wallet->receiveFile( slateFn, identifier );

                // Continue at onReceiveFile
                break;
            }

            if (!errMsg.isEmpty()) {
                reportMessageToUI("Claim request failed", errMsg);
            }

        }
        else {
           reportMessageToUI("Claim request failed", "Unable to process your claim." +
                                  generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) );
        }
        return;
    }

    if ( TAG_RESPONCE_SLATE == tag ) {

        bool success = jsonRespond["success"].toBool(false);
        if ( success ) {
            QString btcAddress = reply->property("param1").toString();

            reportMessageToUI("Your MWC claim succeeded", "Your claim for address " + btcAddress + " was successfully processed.\nPlease note, to finalize transaction might take up to 48 hours to process.");

            backToMainAirDropPage();
        }
        else {
            // error status
            reportMessageToUI("Claim request failed", "Unable to finalize claim process." +
                                                      generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) + "\n\n" +
                                                      "Please note, your last transaction will not be confirmed and its "
                                                      "balance will not be spendable. To fix that please go to "
                                                      "'Transaction' page and cancel the last file based transaction. "
                                                      "Then you will be able to initiate a new claim when this error is resolved.");
        }
        return;
    }

    if ( tag.startsWith(TAG_STATUS_PREF)) {
        // Checking the index
        int idx = tag.mid( TAG_STATUS_PREF.length() ).toInt();

        int errCode = jsonRespond["code"].toInt(-1);
        int64_t amount = jsonRespond["amount"].toString("-1").toLongLong();
        QString status = jsonRespond["status"].toString();
        QString message = jsonRespond["response"].toString();

        if (status.isEmpty())
            status = message;

        Q_ASSERT( idx>=0 && idx<airdropRequests.size() );
        if ( idx>=0 && idx<airdropRequests.size() ) {
            // updating and request next if needed
            for (auto b : bridge::getBridgeManager()->getAirdrop()) {
                b->updateClaimStatus( idx, airdropRequests[idx].btcAddress, status, message, amount, errCode);
                // Request next...
                if (idx + 1 < airdropRequests.size())
                     requestStatusFor(idx + 1);
            }
        }
        return;
    }

}


void Airdrop::onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPassword) {

    if (bridge::getBridgeManager()->getAirdrop().isEmpty())
        return; // Not Airdrop workflow

    if (success) {
        sendRequest( HTTP_CALL::GET, "/v1/getChallenge" ,
                              {"btcaddress", btcaddress, "password", airDropAccPassword, "pubkey", publicKey },
                              "",
                              TAG_GET_CHALLENGE, btcaddress, identifier );
    }
    else {
        reportMessageToUI("Claim request failed", "Unable to start claim process.\n" +
                                                  errorMessage );
    }
}




static void cleanFiles(const QString & inFileName, const QString & outFn) {
    if (!inFileName.isEmpty())
        QFile::remove( inFileName );

    if (!outFn.isEmpty())
        QFile::remove( outFn );
}

// Continue at onReceiveFile
void Airdrop::onReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn ) {
    // It is a global listener. Not necessary it is our request. Need to check first

    QString btcAddress = claimRequests.value( inFileName, "" );
    if (btcAddress.isEmpty())
        return; // not our call, just sckipping

    // It is our call. Letch prepare for the request

    if (!success) {
        // Cleaning the data first
        cleanFiles(inFileName, outFn);

        reportMessageToUI("Claim request failed", "Unable to process the slate with mwc713 to complete the claim.\n" +
                util::formatErrorMessages(errors) );
        return;
    }
    else {
        // outFN should contain the resulting slate...

        QFile slateFile(outFn);
        QByteArray stateData;
        if (slateFile.open(QIODevice::ReadOnly )) {
            stateData = slateFile.readAll();
        }
        else {
            cleanFiles(inFileName, outFn);
            reportMessageToUI("Claim request failed", "Unable to process the slate with mwc713 to complete the claim." );
            return;
        }
        slateFile.close();

        cleanFiles(inFileName, outFn);

        sendRequest( HTTP_CALL::POST, "/v1/submitResponseSlate" ,
                    {"btcaddress", btcAddress },
                     stateData,
                     TAG_RESPONCE_SLATE, btcAddress );
    }
}


// Respond with error to UI. UI expected to stop waiting
void Airdrop::reportMessageToUI( QString title, QString message ) {
    for (auto b : bridge::getBridgeManager()->getAirdrop())
        b->reportMessage(title, message);
}


}

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

#include "h_hodl.h"
#include "../wallet/wallet.h"
#include "windows/h_hodl_w.h"
#include "windows/h_hodlcold_w.h"
#include "windows/h_hodlnode_w.h"
#include "windows/h_hodlclaim_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/HodlStatus.h"
#include "../core/Notification.h"
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QNetworkReply>
#include "../util/Log.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <control/messagebox.h>
#include <QFile>
#include "../util/crypto.h"
#include "../dialogs/h_hodlgetsignature.h"

namespace state {

static const QString TAG_GET_HODL_STATUS      = "hodl_getHODLStatus";
static const QString TAG_CHECK_OUTPUTS        = "hodl_checkOutputs";
static const QString TAG_GET_HODL_REWARD      = "hodl_getPendingHODLRewards";
static const QString TAG_GET_CHALLENGE        = "hodl_getChallenge";
static const QString TAG_REGISTER_HODL        = "hodl_registerHODL";
static const QString TAG_GET_CLAIM_CHALLENGE  = "hodl_getPaymentChallenge";
static const QString TAG_CLAIM_MWC_HODL       = "hodl_claimMWCHODL";
static const QString TAG_RESPONCE_SLATE       = "hodl_submitResponseSlate";

Hodl::Hodl(StateContext * context) :
        State(context, STATE::HODL)
{
    nwManager = new QNetworkAccessManager(this);

    connect( nwManager, &QNetworkAccessManager::finished,        this, &Hodl::replyFinished,   Qt::QueuedConnection );
    connect( context->wallet , &wallet::Wallet::onReceiveFile,   this, &Hodl::onReceiveFile,   Qt::QueuedConnection );
    connect( context->wallet , &wallet::Wallet::onGetNextKeyResult, this, &Hodl::onGetNextKeyResult, Qt::QueuedConnection );
    connect( context->wallet , &wallet::Wallet::onLoginResult,   this, &Hodl::onLoginResult,   Qt::QueuedConnection );
    connect( context->wallet , &wallet::Wallet::onRootPublicKey, this, &Hodl::onRootPublicKey, Qt::QueuedConnection );

    connect( context->hodlStatus, &core::HodlStatus::onHodlStatusWasChanged, this, &Hodl::onHodlStatusWasChanged, Qt::QueuedConnection );
}

Hodl::~Hodl() {}

NextStateRespond Hodl::execute() {
    if (context->appContext->getActiveWndState() != STATE::HODL)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    moveToStartHODLPage();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Hodl::moveToClaimPage(const QString & coldWalletHash) {
    hodlClaimWnd = (wnd::HodlClaim *)context->wndManager->switchToWindowEx( mwc::PAGE_HODL_CLAIM,
                        new wnd::HodlClaim( context->wndManager->getInWndParent(), this, coldWalletHash ) );
}

void Hodl::moveToStartHODLPage() {
    // Request HODL details once more for refresh
    onLoginResult(true);

    if (config::isOnlineWallet()) {
        hodlNormWnd = (wnd::Hodl *) context->wndManager->switchToWindowEx(mwc::PAGE_HODL,
                      new wnd::Hodl(context->wndManager->getInWndParent(),this));
    } else if (config::isOnlineNode()) {
        hodlNodeWnd = (wnd::HodlNode *) context->wndManager->switchToWindowEx(mwc::PAGE_HODL,
                      new wnd::HodlNode(context->wndManager->getInWndParent(),this));
    }
    else if (config::isColdWallet()) {
        hodlColdWnd = (wnd::HodlCold *) context->wndManager->switchToWindowEx(mwc::PAGE_HODL,
                      new wnd::HodlCold(context->wndManager->getInWndParent(),this));
        context->wallet->getRootPublicKey("");
    }
    else {
        Q_ASSERT(false);
    }
}

void Hodl::requestHodlInfoRefresh(const QString & hash) {
    hodlUrl = (context->wallet->getWalletConfig().getNetwork() == "Mainnet") ?
                 config::getHodlMainNetUrl() : config::getHodlTestNetUrl();

    sendRequest( HTTP_CALL::GET, "/v1/getHODLStatus", {}, "", TAG_GET_HODL_STATUS, hash );

    if (hash.isEmpty()) {
        if (config::isOnlineWallet()) {
            hodlWorkflow = HODL_WORKFLOW::INIT;
            Q_ASSERT(config::isOnlineWallet() || config::isColdWallet());
            context->wallet->getRootPublicKey("");
            // continue at  onRootPublicKey( QString rootPubKey, QString message, QString signature )
        }
    }
}


// Set cold wallet public key. That can initiate all status workflow.
void Hodl::setColdWalletPublicKey(QString pubKey) {
    if (pubKey.isEmpty()) {
        context->hodlStatus->setRootPubKey("");
        return;
    }

    Q_ASSERT(config::isOnlineNode());
    Q_ASSERT(crypto::isPublicKeyValid(pubKey));
    if (crypto::isPublicKeyValid(pubKey)) {
        context->hodlStatus->setRootPubKey(pubKey);
        retrieveHodlBalance("");
    }
}

// Request registration for HODL program
void Hodl::registerAccountForHODL() {

    if ( control::MessageBox::RETURN_CODE::BTN2 != control::MessageBox::questionText(nullptr, "HODL Registration",
                                      "Registering for the HODL Program requires the wallet's root public key. This enables outputs and values for the wallet instance to be tracked. Do you wish to continue?",
                                      "Reject", "Accept", false, true) )
    {
        hideWaitingStatus();
        return;
    }

    startChallengeWorkflow(HODL_WORKFLOW::REGISTER);
    // Continue at TAG_GET_CHALLENGE
}

void Hodl::claimMWC(const QString & hash) {

    // Search for Claim ID...

    QVector<core::HodlClaimStatus> claims = context->hodlStatus->getClaimsRequestStatus(hash);
    core::HodlClaimStatus claimNow;
    for (const auto & cl : claims) {
        if (cl.status < 3) {
            claimNow = cl;
            break;
        }
    }

    if (claimNow.claimId<0) {
        control::MessageBox::messageText(nullptr,"HODL Claims", "We don't see any claims that your can claim.");
        hideWaitingStatus();
        return;
    }

    // We want to lock record by status during processing
    context->hodlStatus->lockClaimsRequestStatus(hash, claimNow.claimId);

    hodlWorkflow = HODL_WORKFLOW::CLAIM;
    claimHash = hash;
    claimId = claimNow.claimId; // temp for claiming. Using it because we edon't have cookies for our requests.
    claimAmount = claimNow.amount;

    // First requesting pub key for new transaction. It is already needed for getPaymentChallenge
    context->wallet->getNextKey( claimAmount, "", "" );
    // Continue at onGetNextKeyResult
}

void Hodl::startChallengeWorkflow(HODL_WORKFLOW workflow ) {
    // Public key expected to be ready.
    QString rootPublicKey = context->hodlStatus->getRootPubKey();
    if (rootPublicKey.isEmpty()) {
        Q_ASSERT(false);
        control::MessageBox::messageText(nullptr, "HODL Registration", "We are not finished collecting data from the mwc713 wallet. Please try register for HODL later.");
        return;
    }

    hodlWorkflow = workflow;

    QVector<QString> params{"root_pub_key", rootPublicKey};

    sendRequest( HTTP_CALL::GET, "/v1/getHODLChallenge", params, "", TAG_GET_CHALLENGE );
    // Continue at TAG_GET_CHALLENGE
}


void Hodl::onLoginResult(bool ok) {
    if (!ok)
        return;

    requestHodlInfoRefresh(""); // login - it is current hot wallet only
}


// request message to sign, respond expected to be delivered to the window
void Hodl::requestSignMessage(const QString & message) {
    Q_ASSERT(!message.isEmpty());
    Q_ASSERT(config::isColdWallet());
    Q_ASSERT(config::isOnlineWallet() || config::isColdWallet());
    context->wallet->getRootPublicKey(message);
    // continue at  onRootPublicKey( QString rootPubKey, QString message, QString signature )
}


void Hodl::retrieveHodlBalance(const QString & hash) {
    QString rootPubKey = context->hodlStatus->getRootPubKey();

    if (rootPubKey.isEmpty()) {
        Q_ASSERT(false);
        return;
    }

    if (!context->hodlStatus->isHodlServerActive())
        return;

    const QString hashVal = context->hodlStatus->getHash(hash);

    if (!context->hodlStatus->hasHodlOutputs() ) {
        sendRequest( HTTP_CALL::GET, "/v1/checkOutputs",
                     {"root_pub_key_hash", hashVal }, "", TAG_CHECK_OUTPUTS, hashVal );
    }

    if (!context->hodlStatus->hasAmountToClaim() ) {
        sendRequest( HTTP_CALL::GET, "/v1/getPendingHODLRewards",
                     {"root_pub_key_hash", hashVal }, "", TAG_GET_HODL_REWARD, hashVal );
    }
}

void Hodl::onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature ) {
    if (!success) {
        // Let's rise the error.
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, errMsg );
        hideWaitingStatus();
        resetClaimState();
        return;
    }

    // Cold wallet - just display the signature
    if (config::isColdWallet()) {
        if ( hodlColdWnd ) {
            hodlColdWnd->setRootPubKeyWithSignature(rootPubKey, message,  signature);
        }
        return;
    }

    // Define the work path...
    switch (hodlWorkflow) {
        case HODL_WORKFLOW::INIT: {
            Q_ASSERT(message.isEmpty());
            Q_ASSERT(signature.isEmpty());

            // We want always refresh the HODL status. This optimization doesn't worth it. Normally it is a manual update
            // So we don't want have here checking if we can skip request to HODL server.
            context->hodlStatus->setRootPubKey(rootPubKey);

            if (context->hodlStatus->isHodlServerActive()) {
                if (!context->hodlStatus->hasHodlOutputs()) {
                    sendRequest(HTTP_CALL::GET, "/v1/checkOutputs",
                                {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash()}, "", TAG_CHECK_OUTPUTS,
                                "");
                }

                if (!context->hodlStatus->hasAmountToClaim()) {
                    sendRequest(HTTP_CALL::GET, "/v1/getPendingHODLRewards",
                                {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash()}, "",
                                TAG_GET_HODL_REWARD, "");
                }
            }
            break;
        }
        case HODL_WORKFLOW::REGISTER: {
            Q_ASSERT( rootPubKey == context->hodlStatus->getRootPubKey() );

            Q_ASSERT(!signature.isEmpty());
            Q_ASSERT(!message.isEmpty());
            sendRequest( HTTP_CALL::GET, "/v1/registerHODL",
                         {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash(),
                          "signature", signature,
                          "challenge", message},
                         "", TAG_REGISTER_HODL );
            // TO be continue at respond step
            break;
        }
        case HODL_WORKFLOW::CLAIM: {
            Q_ASSERT( rootPubKey == context->hodlStatus->getRootPubKey() );
            Q_ASSERT(!signature.isEmpty());
            Q_ASSERT(!message.isEmpty());

            Q_ASSERT(config::isOnlineWallet() || config::isOnlineNode());
            Q_ASSERT( claimChallenge == message );
            claimSignature = signature;

            // Now we can claim...
            QVector<QString> params {"root_pub_key_hash", context->hodlStatus->getHash(claimHash),
                                     "claim_id", QString::number(claimId),
                                     "pubkey", claimNextTransPubKey,
                                     "challenge", claimChallenge,
                                     "signature", claimSignature
                                     };

            sendRequest( HTTP_CALL::GET, "/v1/claimMWCHODL",
                         params, "", TAG_CLAIM_MWC_HODL );

            break;
        }
        default: {
            Q_ASSERT(false);
        }
    }
}

void Hodl::onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPassword) {
    // Will be empty values.
    Q_UNUSED(btcaddress)
    Q_UNUSED(airDropAccPassword)

    if (hodlClaimWnd==nullptr)
        return; // Not our case, dropping request...

    if (success) {
        Q_ASSERT(hodlWorkflow == HODL_WORKFLOW::CLAIM);
        Q_ASSERT(claimId >= 0);
        Q_ASSERT(claimAmount > 0);

        claimNextTransIdentifier = identifier;
        claimNextTransPubKey = publicKey;

        // Requesting challenge...
        QVector<QString> params{"root_pub_key_hash", context->hodlStatus->getHash(claimHash),
                                "pubkey", claimNextTransPubKey,
                                "claim_id", QString::number(claimId)};
        sendRequest( HTTP_CALL::GET, "/v1/getPaymentChallenge", params, "", TAG_GET_CLAIM_CHALLENGE, QString::number(claimId) );
    }
    else {
        reportMessageToUI("Claim request failed", "Unable to start claim process.\n" +
                                                  errorMessage );
    }
}


///////////////////////////////////////////////

void Hodl::sendRequest(HTTP_CALL call, const QString & api,
                          const QVector<QString> & params,
                          const QByteArray & body, //
                          const QString & tag, const QString & param1, const QString & param2,
                          const QString & param3, const QString & param4) {

    QString url = hodlUrl + api;

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
    logger::logInfo("HODL", "Requesting: " + url );
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

void Hodl::replyFinished(QNetworkReply* reply) {
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
        logger::logInfo("HODL", "Success respond for Tag: " + tag + "  Reply " + strReply);

        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(strReply.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            requestOk = false;
            requestErrorMessage = tag + " Unable to parse respond Json at position " + QString::number(error.offset) +
                                  "\nJson string: " + strReply;
        }
        jsonRespond = jsonDoc.object();
    }
    else  {
        requestOk = false;
        requestErrorMessage = tag + " " + reply->errorString();
        logger::logInfo("HODL", "Fail respond for Tag: " + tag + "  requestErrorMessage: " + requestErrorMessage);
    }
    reply->deleteLater();

    // Done with reply. Now processing the results by tags
    if ( TAG_GET_HODL_STATUS == tag) {
        if (requestOk) {
            bool success = jsonRespond["success"].toBool(false);
            context->hodlStatus->setHodlStatus( success, jsonRespond["message"].toString(), TAG_GET_HODL_STATUS );
        } else {
            context->hodlStatus->setError( TAG_GET_HODL_STATUS, "Unable to request the status info from " + hodlUrl +
                                    ".\nGet communication error: " + requestErrorMessage);
        }

        QString hash = reply->property("param1").toString();
        if ( !hash.isEmpty() && context->hodlStatus->isHodlServerActive() ) {
            sendRequest( HTTP_CALL::GET, "/v1/checkOutputs",
                         {"root_pub_key_hash", hash }, "", TAG_CHECK_OUTPUTS, hash );

            sendRequest( HTTP_CALL::GET, "/v1/getPendingHODLRewards",
                         {"root_pub_key_hash", hash }, "", TAG_GET_HODL_REWARD, hash );
        }

        return;
    }

    if ( TAG_CHECK_OUTPUTS == tag) {
        if (requestOk) {
            QString hash = reply->property("param1").toString();

            bool success = jsonRespond["success"].toBool(false);
            if (success) {
                //  {"success":true,"outputs_qualified":[{"amount":0.1,"commit":"0939bd49fd3ebecf94846762e164ab83db50ebdb7d1e9b5178e745cfb8fa83caea","weight":1},{"amount":0.3,"commit":"087382855d829290ed292163dd120149bfe08b1cc56822c013b35b26ba5b9a2830","weight":1}]}
                QJsonArray outputs_qualified = jsonRespond["outputs_qualified"].toArray();
                QVector<core::HodlOutputInfo> hodlOutputs;

                for ( const auto outputRef : outputs_qualified ) {
                    auto output = outputRef.toObject();

                    double amount  = output["amount"].toString().toDouble();
                    QString commit = output["commit"].toString();
                    double weight  = output["weight"].toDouble();
                    QString cls    = output["class"].toString();

                    if (!commit.isEmpty())
                        hodlOutputs.push_back( core::HodlOutputInfo::create(commit, amount, weight, cls) );
                }

                context->hodlStatus->setHodlOutputs(hash, true, hodlOutputs, TAG_CHECK_OUTPUTS );
                if (hash.isEmpty())
                    context->hodlStatus->updateRegistrationTime();
            }
            else {
                // {"success": false, "error_code": 1, "error_message": "Please reigster first."}
                int errorCode = jsonRespond["error_code"].toInt();
                QString errorMessage = jsonRespond["error_message"].toString();

                if (errorCode==1) {
                    context->hodlStatus->setHodlOutputs( hash, false, QVector<core::HodlOutputInfo>(), TAG_CHECK_OUTPUTS);
                }
                else if (errorCode==2) {
                    // {"success": false, "error_code": 2, "error_message": "Registration accepted, but not processed."}
                    context->hodlStatus->setHodlOutputs( hash, true, QVector<core::HodlOutputInfo>(), TAG_CHECK_OUTPUTS);
                }
                else {
                    context->hodlStatus->setError( TAG_CHECK_OUTPUTS, tag + " " + errorMessage);
                    notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to get HODL output list. " + errorMessage );
                }
            }
        } else {
            context->hodlStatus->setError( TAG_CHECK_OUTPUTS, "Unable to request the HODL output info from " + hodlUrl +
                                                                    ".\nGet communication error: " + requestErrorMessage);
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to get HODL output list. We can't manage your outputs for sending" );
        }

        return;
    }

    if ( TAG_GET_HODL_REWARD == tag ) {
        if (requestOk) {
            //{\"success\":true,\"payments\":[{\"amount\":\"1000000000\",\"claim_id\":1,\"status\":0},{\"amount\":\"1100000000\",\"claim_id\":2,\"status\":0},{\"amount\":\"1200000000\",\"claim_id\":3,\"status\":0}]}
            // { "success": false, "error_code": 1, "error_message": "Not registered"}
            bool success = jsonRespond["success"].toBool(false);

            QString hash = reply->property("param1").toString();

            QVector<core::HodlClaimStatus> HodlClaimStatus;
            if (success) {
                QJsonArray rewards = jsonRespond["payments"].toArray();
                for (const auto & r : rewards ) {
                    auto rwd = r.toObject();
                    bool ok1 = false;
                    qlonglong amount = rwd["amount"].toString().toLongLong(&ok1);
                    int claim_id = rwd["claim_id"].toInt();
                    int status = rwd["status"].toInt();

                    if (ok1) {
                        core::HodlClaimStatus hodlStatus;
                        hodlStatus.setData(amount, claim_id, status);
                        HodlClaimStatus.push_back(hodlStatus);
                    }
                    else {
                        context->hodlStatus->setError( TAG_GET_HODL_REWARD, "Unable to process HODL reward data. Not found expected data.");
                        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to process Hodl reward data. Not found expected data." );
                        success = false;
                        break;
                    }
                }
            }
            else {
                QString errMsg = jsonRespond["error_message"].toString();
                QString errorCode = jsonRespond["error_code"].toString();

                if (!errMsg.contains("Not registered", Qt::CaseInsensitive)) {
                    QString error = "Unable to retrieve HODL reward data. Error Code: " + errorCode + "; " + errMsg;
                    context->hodlStatus->setError(TAG_GET_HODL_REWARD, error);
                    notify::appendNotificationMessage(notify::MESSAGE_LEVEL::CRITICAL, error);
                }
            }

            if (success)
                context->hodlStatus->setHodlClaimStatus(hash, HodlClaimStatus,TAG_GET_HODL_REWARD);

        } else {
            context->hodlStatus->setError( TAG_GET_HODL_REWARD, "Unable to request available to claim HODL amount info from " + hodlUrl +
                                                            ".\nGet communication error: " + requestErrorMessage);
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to get available to claim HODL amount." );
        }
    }

    if (!requestOk) {
        reportMessageToUI("Network error", "Unable to communicate with " + hodlUrl + ".\nGet communication error: " + requestErrorMessage);
        return;
    }

    if ( TAG_GET_CHALLENGE == tag ) {
        // continue with a registerHODL workflow
        bool success = jsonRespond["success"].toBool(false);
        if ( success ) {
            QString challenge = jsonRespond["challenge"].toString();
            Q_ASSERT(!challenge.isEmpty());
            if (challenge.isEmpty()) {
                resetClaimState();
                reportMessageToUI("HODL request failed", "Unable to get a challenge form the the HODL server" );
                return;
            }

            if (config::isOnlineNode() ) {
                // We can't call wallet, we need to ask user for the Signature...
                dlg::HodlGetSignature signatureDlg(nullptr, challenge);
                if ( signatureDlg.exec() == QDialog::Accepted) {
                    Hodl::onRootPublicKey( true, "",  context->hodlStatus->getRootPubKey(), challenge, signatureDlg.getSignature() );
                }
                else {
                    Hodl::onRootPublicKey( false, "Signature wasn't provided by user", "", "", "" );
                }
                return;
            }
            else {

                // Requesting wallet for the signature. It is true for all workflows...
                Q_ASSERT(config::isOnlineWallet() || config::isColdWallet());
                context->wallet->getRootPublicKey(challenge);
                // Continue at onRootPublicKey
            }
            return;
        }
        else {
            resetClaimState();
            // error status
            reportMessageToUI("HODL request failed", "HODL server getChallenge API returned error:\n" +
                           generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) );
        }
        return;
    }

    if ( TAG_REGISTER_HODL == tag ) {
        hodlWorkflow = HODL_WORKFLOW::INIT;
        bool success = jsonRespond["success"].toBool(false);
        if ( success ) {
            // it is end point. Done...
            reportMessageToUI("HODL registration succeeded",
                    "Congratulations! Your wallet instance is successfully registered for the HODL Program.\n\n"
                    "During next 24 hours you should see if HODL server was able to discover your wallet outputs." );

            // Updating the registration time
            context->hodlStatus->updateRegistrationTime();
            context->hodlStatus->setHodlOutputs( "", true, QVector<core::HodlOutputInfo>(), TAG_CHECK_OUTPUTS);
            return;
        }
        else {
            // error status
            reportMessageToUI("HODL registration failed", "Unable to register account for HODL. " +
                          generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) );
        }
        return;
    }

    if (TAG_GET_CLAIM_CHALLENGE == tag) {
        // getPaymentChallenge
        // {"success": true, "challenge": "HODL_RZLdJf2OvS_f65e48eb86d0a915d42b1f4539024f912ea955a341b124bcc22ee380d2c839a1afa595e4cf6ce63e1dc5be4ac9fd03976f22aa28bbb6c38b3c373cc158e93acc"}
        bool success = jsonRespond["success"].toBool(false);
        if ( success ) {
            QString challenge = jsonRespond["challenge"].toString();

            QString paramClaimId = reply->property("param1").toString();
            Q_ASSERT(!paramClaimId.isEmpty());

            if (paramClaimId != QString::number(claimId) ) {
                resetClaimState();
                if (hodlClaimWnd) {
                    hodlClaimWnd->reportMessage("Internal Error", "Internal data is not complete. we can't continue claiming. Please retry.");
                }
                return;
            }

            qDebug() << "Get Challenge " << challenge << " got claimId " << claimId;

            // Requesting public key with challenge...
            // Setting up the context for
            hodlWorkflow = HODL_WORKFLOW::CLAIM;
            claimChallenge = challenge;

            if (!claimHash.isEmpty() ) {
                // We can't call wallet, we need to ask user for the Signature...
                dlg::HodlGetSignature signatureDlg(nullptr, challenge);
                if ( signatureDlg.exec() == QDialog::Accepted) {
                    Hodl::onRootPublicKey( true, "",  context->hodlStatus->getRootPubKey(), challenge, signatureDlg.getSignature() );
                }
                else {
                    Hodl::onRootPublicKey( false, "Signature wasn't provided by user", "", "", "" );
                }
                return;
            }
            else {
                context->wallet->getRootPublicKey(challenge);
                // Continue on
                // onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature );
            }
        }
        else {
            reportMessageToUI("Claim request failed", "Unable to request a payment challenge." +
                                                      generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )));
        }
        return;
    }

    if (TAG_CLAIM_MWC_HODL == tag) {
        bool success = jsonRespond["success"].toBool(false);

        Q_ASSERT( !claimNextTransIdentifier.isEmpty() && !claimNextTransPubKey.isEmpty() && claimId>=0 );
        if ( claimNextTransIdentifier.isEmpty() || claimNextTransPubKey.isEmpty() || claimId<0 ) {
            if (hodlClaimWnd) {
                hodlClaimWnd->reportMessage("Internal Error", "Internal data is not complete. we can't continue claiming. Please retry.");
            }
            resetClaimState(); // failure case
            return;
        }

        if ( success ) {
            QString errMsg;
            while (true) {
                QString slateBase64 = jsonRespond["slate"].toString();
                // Expected to have a base64 encoded slate
                if (slateBase64.isEmpty()) {
                    errMsg = "Not found slate data from claimMWCHODL API respond";
                    break;
                }

                QByteArray slate = QByteArray::fromBase64(slateBase64.toLatin1());

                if (slate.isEmpty()) {
                    errMsg = "Not able to decode slate from claimMWCHODL API respond";
                    break;
                }

                QString slateFn = ioutils::getAppDataPath("tmp") + "/" + "slate"+QString::number(requestCounter++)+".tx";
                {
                    // Clean up response if exist
                    QString slateRespFn = slateFn + ".response";
                    QFile::remove(slateRespFn);
                }


                QFile slateFile(slateFn);
                if (slateFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    slateFile.write(slate);
                }
                else {
                    errMsg = "Not able to save slate at the file " + slateFn;
                    break;

                }
                slateFile.close();

                // Starting the slate processing transaction...
                // wallet expected to be offline...
                Q_ASSERT(config::isOnlineWallet() || config::isColdWallet());
                context->wallet->setReceiveAccount(context->appContext->getReceiveAccount());
                context->wallet->receiveFile( slateFn, claimNextTransIdentifier );

                // Continue at onReceiveFile
                return;
            }

            Q_ASSERT(!errMsg.isEmpty());
            reportMessageToUI("Claim request failed", errMsg);
        }
        else {
            reportMessageToUI("Claim request failed", "Unable to process your claim." +
                                                      generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) );
        }
        resetClaimState();
        return;
    }

    if ( TAG_RESPONCE_SLATE == tag ) {
        // End point in any case
        QString hash = claimHash;
        resetClaimState();

        bool success = jsonRespond["success"].toBool(false);
        if ( success ) {
            // Let's update the page status.
            requestHodlInfoRefresh(hash);
            reportMessageToUI("HODL claim succeeded", "Your claim for this wallet was successfully processed.\nPlease note that finalization will happen after all claims have been processed.");
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
}

// Reset Claim workflow, so no internal data will exist
void Hodl::resetClaimState() {
    hodlWorkflow = HODL_WORKFLOW::INIT; // It is end point in any case
    claimHash = "";
    claimId = -1; // temp for claiming. Using it because we edon't have cookies for our requests.
    claimAmount = -1;
    claimChallenge = "";
    claimSignature = "";
    claimNextTransIdentifier = "";
    claimNextTransPubKey = "";
}


static void cleanFiles(const QString & inFileName, const QString & outFn) {
    if (!inFileName.isEmpty())
        QFile::remove( inFileName );

    if (!outFn.isEmpty())
        QFile::remove( outFn );
}

// Continue at onReceiveFile
void Hodl::onReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn ) {
    // It is a global listener. Not necessary it is our request. Need to check first
    if (hodlWorkflow != HODL_WORKFLOW::CLAIM)
        return;

    if (!success) {
        resetClaimState();

        // Cleaning the data first
        cleanFiles(inFileName, outFn);

        reportMessageToUI("Claim request failed", "Unable to process the slate with mwc713 to complete the claim.\n" +
                                                  util::formatErrorMessages(errors) );

        return;
    }
    else {
        // outFN should contain the resulting slate...

        QFile slateFile(outFn);
        QByteArray slateData;
        if (slateFile.open(QIODevice::ReadOnly )) {
            slateData = slateFile.readAll();
        }
        else {
            resetClaimState();
            cleanFiles(inFileName, outFn);
            reportMessageToUI("Claim request failed", "Unable to process the slate with mwc713 to complete the claim." );
            return;
        }
        slateFile.close();

        cleanFiles(inFileName, outFn);

        sendRequest( HTTP_CALL::POST, "/v1/postHODLResponseSlate" ,
                     {"claim_id", QString::number(claimId),
                      "root_pub_key_hash", context->hodlStatus->getHash(claimHash) },
                      slateData,
                      TAG_RESPONCE_SLATE );
    }

}


void Hodl::onHodlStatusWasChanged() {
    if (hodlNormWnd)
        hodlNormWnd->updateHodlState();
    else if (hodlNodeWnd)
        hodlNodeWnd->updateHodlState();
    else if (hodlClaimWnd)
        hodlClaimWnd->updateHodlState();
}

// Respond with error to UI. UI expected to stop waiting
void Hodl::reportMessageToUI( const QString & title, const QString & message ) {
    if (hodlNormWnd)
        hodlNormWnd->reportMessage(title, message);
    else if (hodlNodeWnd)
        hodlNodeWnd->reportMessage(title, message);
    else if (hodlColdWnd)
        hodlColdWnd->reportMessage(title, message);
    else if (hodlClaimWnd)
        hodlClaimWnd->reportMessage(title, message);
}

void Hodl::hideWaitingStatus() {
    if (hodlNormWnd)
        hodlNormWnd->hideWaitingStatus();
    else if (hodlNodeWnd)
        hodlNodeWnd->hideWaitingStatus();
    else if (hodlColdWnd)
        hodlColdWnd->hideWaitingStatus();
}


QVector<int> Hodl::getColumnsWidhts() {
    return context->appContext->getIntVectorFor("HodlTblWidth");
}

void Hodl::updateColumnsWidhts(QVector<int> widths) {
    context->appContext->updateIntVectorFor("HodlTblWidth", widths);
}



}

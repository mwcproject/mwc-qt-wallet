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

static const QString TAG_GET_HODL_STATUS      = "getHODLStatus";
static const QString TAG_CHECK_OUTPUTS        = "checkOutputs";
//static const QString TAG_GET_AMOUNT           = "getAmount";
static const QString TAG_GET_CHALLENGE        = "getChallenge";
static const QString TAG_REGISTER_HODL        = "registerHODL";
static const QString TAG_CLAIM_MWC            = "claimMWC";
static const QString TAG_RESPONCE_SLATE       = "submitResponseSlate";

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

void Hodl::moveToClaimPage() {
    hodlClaimWnd = (wnd::HodlClaim *)context->wndManager->switchToWindowEx( mwc::PAGE_HODL_CLAIM,
                        new wnd::HodlClaim( context->wndManager->getInWndParent(), this ) );
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
        retrieveHodlBalance();
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

    // REST API workflow...
    // https://<api_endpoint>/v1/getChallenge?root_pub_key=<pubkey>
    // https://<api_endpoint>/v1/registerHODL?root_pub_key_hash=<pubkeyhash>&signature=<signature>&challenge=<challenge>

    startChallengeWorkflow(HODL_WORKFLOW::REGISTER, "", -1);
    // Continue at TAG_GET_CHALLENGE
}

void Hodl::claimMWC() {

    control::MessageBox::messageText(nullptr,"HODL Claims", "Sorry, your wallet doesn't support claiming for HODL program. Please upgrade it to the latest version.");
    return;

/*    if (  !context->hodlStatus->hasAmountToClaim()) {
        control::MessageBox::messageText(nullptr, "HODL Claim", "No HODL reward found to be claimed.");
        return;
    }

    // Public key expected to be ready,
    hodlWorkflow = HODL_WORKFLOW::CLAIM;

    Q_ASSERT(config::isOnlineWallet() || config::isColdWallet());
    context->wallet->getNextKey( context->hodlStatus->getAmountToClaim(), "", "" );
    // Continue at onGetNextKeyResult
    */
}

void Hodl::startChallengeWorkflow(HODL_WORKFLOW workflow, QString publicKey, int64_t claimAmount ) {
    // Public key expected to be ready.
    QString rootPublicKey = context->hodlStatus->getRootPubKey();
    if (rootPublicKey.isEmpty()) {
        Q_ASSERT(false);
        control::MessageBox::messageText(nullptr, "HODL Registration", "We are not finished collecting data from the mwc713 wallet. Please try register for HODL later.");
        return;
    }

    hodlWorkflow = workflow;

    QVector<QString> params{"root_pub_key", rootPublicKey};

    if (!publicKey.isEmpty()) {
        Q_ASSERT( hodlWorkflow == HODL_WORKFLOW::CLAIM );

        params.push_back("pubkey");
        params.push_back(publicKey);

        params.push_back("claimAmount");
        params.push_back( QString::number(claimAmount) );
    }

    // Currently doing and checking registration only. getHODLChallenge support registration only..
    Q_ASSERT(publicKey.isEmpty());

    sendRequest( HTTP_CALL::GET, "/v1/getHODLChallenge", params, "", TAG_GET_CHALLENGE );
    // Continue at TAG_GET_CHALLENGE
}


void Hodl::onLoginResult(bool ok) {
    Q_UNUSED(ok)

    hodlUrl = (context->wallet->getWalletConfig().getNetwork() == "Mainnet") ?
                 config::getHodlMainNetUrl() : config::getHodlTestNetUrl();

    sendRequest( HTTP_CALL::GET, "/v1/getHODLStatus", {}, "", TAG_GET_HODL_STATUS );

    if (config::isOnlineWallet()) {
        hodlWorkflow = HODL_WORKFLOW::INIT;
        Q_ASSERT(config::isOnlineWallet() || config::isColdWallet());
        context->wallet->getRootPublicKey("");
        // continue at  onRootPublicKey( QString rootPubKey, QString message, QString signature )
    }
}


// request message to sign, respond expected to be delivered to the window
void Hodl::requestSignMessage(const QString & message) {
    Q_ASSERT(!message.isEmpty());
    Q_ASSERT(config::isColdWallet());
    Q_ASSERT(config::isOnlineWallet() || config::isColdWallet());
    context->wallet->getRootPublicKey(message);
    // continue at  onRootPublicKey( QString rootPubKey, QString message, QString signature )
}


void Hodl::retrieveHodlBalance() {
    QString rootPubKey = context->hodlStatus->getRootPubKey();

    if (rootPubKey.isEmpty()) {
        Q_ASSERT(false);
        return;
    }


    if (!context->hodlStatus->hasHodlOutputs() ) {
        sendRequest( HTTP_CALL::GET, "/v1/checkOutputs",
                     {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash() }, "", TAG_CHECK_OUTPUTS );
    }

/*    if (!context->hodlStatus->hasAmountToClaim() ) {
        sendRequest( HTTP_CALL::GET, "/v1/getAmountHODL",
                     {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash() }, "", TAG_GET_AMOUNT );
    }*/
}

void Hodl::onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature ) {
    if (!success) {
        // Let's rise the error.
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, errMsg );
        hideWaitingStatus();
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

            context->hodlStatus->setRootPubKey(rootPubKey);

            if (!context->hodlStatus->hasHodlOutputs() ) {
                sendRequest( HTTP_CALL::GET, "/v1/checkOutputs",
                             {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash() }, "", TAG_CHECK_OUTPUTS );
            }

            /*if (!context->hodlStatus->hasAmountToClaim() ) {
                sendRequest( HTTP_CALL::GET, "/v1/getAmountHODL",
                             {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash() }, "", TAG_GET_AMOUNT );
            }*/
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

            sendRequest( HTTP_CALL::GET, "/v1/claimMWC",
                         {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash(),
                          "signature", signature,
                          "challenge", message},
                         "", TAG_CLAIM_MWC,
                         message, signature);
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

    if (success) {
        Q_ASSERT(hodlWorkflow == HODL_WORKFLOW::CLAIM);

        claimNextTransIdentifier = identifier;
        claimNextTransPubKey = publicKey;

        startChallengeWorkflow( HODL_WORKFLOW::CLAIM, publicKey, context->hodlStatus->getAmountToClaim() );
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
            context->hodlStatus->setHodlStatus( jsonRespond["message"].toString(), TAG_GET_HODL_STATUS );
        } else {
            context->hodlStatus->setError( TAG_GET_HODL_STATUS, "Unable to request the status info from " + hodlUrl +
                                    ".\nGet communication error: " + requestErrorMessage);
        }

        return;
    }

    if ( TAG_CHECK_OUTPUTS == tag) {
        if (requestOk) {
            bool success = jsonRespond["success"].toBool(false);
            if (success) {
                //  {"success":true,"outputs_qualified":[{"amount":0.1,"commit":"0939bd49fd3ebecf94846762e164ab83db50ebdb7d1e9b5178e745cfb8fa83caea","weight":1},{"amount":0.3,"commit":"087382855d829290ed292163dd120149bfe08b1cc56822c013b35b26ba5b9a2830","weight":1}]}
                QJsonArray outputs_qualified = jsonRespond["outputs_qualified"].toArray();
                QVector<core::HodlOutputInfo> hodlOutputs;

                for ( const auto outputRef : outputs_qualified ) {
                    auto output = outputRef.toObject();

                    double amount = output["amount"].toDouble();
                    QString commit = output["commit"].toString();
                    double weight = output["weight"].toDouble();

                    if (!commit.isEmpty())
                        hodlOutputs.push_back( core::HodlOutputInfo::create(commit, amount, weight) );
                }

                context->hodlStatus->setHodlOutputs( true, hodlOutputs, TAG_CHECK_OUTPUTS );
                context->hodlStatus->updateRegistrationTime();
            }
            else {
                // {"success": false, "error_code": 1, "error_message": "Please reigster first."}
                int errorCode = jsonRespond["error_code"].toInt();
                QString errorMessage = jsonRespond["error_message"].toString();

                if (errorCode==1) {
                    context->hodlStatus->setHodlOutputs( false, QVector<core::HodlOutputInfo>(), TAG_CHECK_OUTPUTS);
                }
                else if (errorCode==2) {
                    // {"success": false, "error_code": 2, "error_message": "Registration accepted, but not processed."}
                    context->hodlStatus->setHodlOutputs( true, QVector<core::HodlOutputInfo>(), TAG_CHECK_OUTPUTS);
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

    /*if ( TAG_GET_AMOUNT == tag ) {
        if (requestOk) {
            Q_ASSERT(false); // Implement me json to amount + period
            context->hodlStatus->setClaimAmount( 0L, TAG_GET_AMOUNT );
        } else {
            context->hodlStatus->setError( TAG_GET_AMOUNT, "Unable to request available to claim HODL amount info from " + hodlUrl +
                                                            ".\nGet communication error: " + requestErrorMessage);
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to get available to claim HODL amount." );
        }
    }*/

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
                hodlWorkflow = HODL_WORKFLOW::INIT;
                reportMessageToUI("HODL request failed", "Unable to get a challenge form the the HODL server" );
                return;
            }

            if (config::isOnlineNode()) {
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
            hodlWorkflow = HODL_WORKFLOW::INIT;
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
            context->hodlStatus->setHodlOutputs( true, QVector<core::HodlOutputInfo>(), TAG_CHECK_OUTPUTS);
            return;
        }
        else {
            // error status
            reportMessageToUI("HODL registration failed", "Unable to register account for HODL. " +
                          generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) );
        }
        return;
    }

    if (TAG_CLAIM_MWC == tag) {
        bool success = jsonRespond["success"].toBool(false);

        QString challendge = reply->property("param1").toString();
        QString signature = reply->property("param2").toString();

        QString rootPublicKey = context->hodlStatus->getRootPubKey();
        Q_ASSERT( !rootPublicKey.isEmpty() && !challendge.isEmpty() && !signature.isEmpty() && !claimNextTransIdentifier.isEmpty() );

        if ( success && !rootPublicKey.isEmpty() && !challendge.isEmpty() && !signature.isEmpty() && !claimNextTransIdentifier.isEmpty() ) {
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

            hodlWorkflow = HODL_WORKFLOW::INIT; // failure case
            Q_ASSERT(!errMsg.isEmpty());
            reportMessageToUI("Claim request failed", errMsg);
        }
        else {
            hodlWorkflow = HODL_WORKFLOW::INIT;
            reportMessageToUI("Claim request failed", "Unable to process your claim." +
                                                      generateMessage(jsonRespond["error_message"].toString(), jsonRespond["error_code"].toInt( INT_MAX )) );
        }
        hodlWorkflow = HODL_WORKFLOW::INIT;
        return;
    }

    if ( TAG_RESPONCE_SLATE == tag ) {
        hodlWorkflow = HODL_WORKFLOW::INIT; // It is end point in any case

        bool success = jsonRespond["success"].toBool(false);
        if ( success ) {
            // Let's update the page status.
            onLoginResult(true);

            reportMessageToUI("HODL claim succeeded", "Your claim for this wallet was successfully processed.\nPlease note, to finalize transaction might take up to 48 hours to process.");

            moveToStartHODLPage();
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
        hodlWorkflow = HODL_WORKFLOW::INIT;

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
            hodlWorkflow = HODL_WORKFLOW::INIT;
            cleanFiles(inFileName, outFn);
            reportMessageToUI("Claim request failed", "Unable to process the slate with mwc713 to complete the claim." );
            return;
        }
        slateFile.close();

        cleanFiles(inFileName, outFn);

        sendRequest( HTTP_CALL::POST, "/v1/submitResponseSlate" ,
                     {"root_pub_key_hash", context->hodlStatus->getRootPubKeyHash() },
                     stateData,
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

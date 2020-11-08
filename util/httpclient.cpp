// Copyright 2020 The MWC Developers
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

#include "httpclient.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QDataStream>
#include "stringutils.h"
#include "Log.h"

namespace util {

HttpClient::HttpClient() {
    nwManager = new QNetworkAccessManager(this);
    connect(nwManager, &QNetworkAccessManager::finished, this, &HttpClient::replyFinished, Qt::QueuedConnection);
}

void HttpClient::sendRequest(HTTP_CALL call, const QString & url,
                          const QString &tag,
                          const QVector<QString> & params,
                          const QByteArray & body, //
                          const QString & param1, const QString & param2,
                          const QString & param3, const QString & param4) {

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

void HttpClient::replyFinished(QNetworkReply* reply) {
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
        logger::logInfo("HttpClient", "Success respond for Tag: " + tag + "  Reply " + strReply);

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
        logger::logInfo("HttpClient", "Fail respond for Tag: " + tag + "  requestErrorMessage: " + requestErrorMessage);
    }
    reply->deleteLater();

    // Done with reply. Now processing the results by tags

    onProcessHttpResponse(requestOk, tag, jsonRespond,
                reply->property("param1").toString(),
                reply->property("param2").toString(),
                reply->property("param3").toString(),
                reply->property("param4").toString()
            );
}


}

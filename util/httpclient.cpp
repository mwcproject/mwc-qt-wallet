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
#include <QFuture>
#include <QtConcurrent>
#include <QTimer>
#include <QThread>
#include <QThreadPool>

#include "stringutils.h"
#include "Log.h"
#include "ui_x_events.h"

namespace util {

static QThreadPool * httpClientThreadPool() {
    static QThreadPool pool;
    static bool inited = false;
    if (!inited) {
        pool.setMaxThreadCount(2);
        pool.setExpiryTimeout(-1); // keep threads alive for reuse
        inited = true;
    }
    return &pool;
}

HttpClient::HttpClient(QObject * parent) : QObject(parent) {
    nwManager = new QNetworkAccessManager(this);
}

QString HttpClient::sendRequest(HTTP_CALL call, const QString & url,
                          const QVector<QString> & queryParams, // key/value
                          const QVector<QString> & headers, // key/value
                          const QByteArray & body,
                          int timeoutMs,
                          bool logRequest) {

    qDebug() << "Sending request: " << url << ", params: " << queryParams << ",  headers: " << headers;

    if (logRequest) {
        logger::logInfo( logger::HTTP_CLIENT, "Requesting: " + url + "   with body: " + util::string2shortStrR( QString::fromUtf8(body), 50) );
    }

    QFuture<QString> request = QtConcurrent::run(httpClientThreadPool(), [=]() -> QString {
        // Per-call manager in this thread
        QNetworkAccessManager nam;

        QUrl requestUrl(url);

        // enrich with params
        Q_ASSERT( queryParams.size()%2==0 );
        QUrlQuery query;
        for (int t=1; t<queryParams.size(); t+=2) {
            query.addQueryItem( util::urlEncode(queryParams[t-1]), util::urlEncode(queryParams[t]));
        }
        // Note: QT encoding has issues, some symbols will be skipped.
        // No encoding needed because we encode params with out code.
        requestUrl.setQuery(query.query(QUrl::PrettyDecoded), QUrl::StrictMode );

        QNetworkRequest request(requestUrl);

        // sslLibraryVersionString neede as a workaroung for a deadlock at defaultConfiguration, qt v5.9
        QSslSocket::sslLibraryVersionString();
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        config.setProtocol(QSsl::TlsV1_2);
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(config);

        Q_ASSERT( headers.size()%2==0 );
        for (int t=1; t<headers.size(); t+=2) {
            request.setRawHeader(headers[t-1].toUtf8(), headers[t].toUtf8());
        }

        QNetworkReply* reply = nullptr;
        if (call == GET) reply = nam.get(request);
        else if (call == POST) reply = nam.post(request, body);
        else Q_ASSERT(false);

        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        timer.start(timeoutMs);
        loop.exec();

        QString response;
        bool finished = reply->isFinished();
        auto err=reply->error();
        if (finished && err == QNetworkReply::NoError) {
            response = reply->readAll().trimmed();
        }
        reply->deleteLater();
        return response;
    });

    QString response = request.result();
    if (response.isEmpty()) {
        logger::logError( logger::HTTP_CLIENT, "Requesting to " + url + "  is failed" );
    }
    return response;
}


}

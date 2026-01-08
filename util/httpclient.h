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

#ifndef MWC_QT_WALLET_HTTPCLIENT_H
#define MWC_QT_WALLET_HTTPCLIENT_H

#include <QString>
#include <QByteArray>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace util {

// Http client, SYNC and must be run in QT thread with event loop
class HttpClient : public QObject {
Q_OBJECT
public:
    enum HTTP_CALL {
        GET, POST
    };

    HttpClient(QObject * parent);

    // Sync call, will wait for the response. Call from another QT thread with a loop events
    QString sendRequest(HTTP_CALL call, const QString & url,
                              const QVector<QString> & queryParams, // key/value
                              const QVector<QString> & headers, // key/value
                              const QByteArray & body,
                              int timeoutMs,
                              bool logRequest);
private:
    QNetworkAccessManager * nwManager = nullptr;
};

}

#endif //MWC_QT_WALLET_HTTPCLIENT_H

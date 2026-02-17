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
#include <QMutex>
#include <QHash>
#include <QUrl>

class QSslSocket;

namespace util {

// Thread-safe synchronous HTTP client for POST requests.
class HttpClient {
public:
    enum HTTP_CALL {
        POST
    };

    explicit HttpClient(const QString & baseUrl);
    ~HttpClient();

    // Sync call, waits for response. Safe to call from multiple threads.
    QString sendRequest(HTTP_CALL call, const QString & path,
                              const QVector<QString> & queryParams, // key/value
                              const QVector<QString> & headers, // key/value
                              const QByteArray & body,
                              int timeoutMs,
                              bool logRequest);

private:
    bool ensureConnectedLocked(int timeoutMs);
    void closeSocketLocked();
    QString buildRequestTarget(const QString & path,
                               const QVector<QString> & queryParams) const;
    bool readResponseLocked(int timeoutMs, int &statusCode,
                            QHash<QByteArray, QByteArray> & responseHeaders,
                            QByteArray & responseBody);

private:
    Q_DISABLE_COPY(HttpClient)

    QMutex socketMutex;
    QSslSocket * socket = nullptr;
    QString baseHost;
    quint16 basePort = 0;
    bool baseTls = false;
};

}

#endif //MWC_QT_WALLET_HTTPCLIENT_H

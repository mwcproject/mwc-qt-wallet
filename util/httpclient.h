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
#include <QHash>
#include <QObject>
#include <QUrl>
#include <QVector>

class QSslSocket;
class QThread;

namespace util {

enum HTTP_CALL {
    POST
};

// Thread-safe synchronous HTTP client for POST requests.
class HttpClientSyncImpl : public QObject {
    Q_OBJECT
public:
    explicit HttpClientSyncImpl(const QString & baseUrl, QObject * parent = nullptr);
    ~HttpClientSyncImpl() override;

    // Sync call. Must be executed from HttpClientSyncImpl's owning thread.
    QString sendRequest(HTTP_CALL call, const QString & path,
                              const QVector<QString> & queryParams, // key/value
                              const QVector<QString> & headers, // key/value
                              const QByteArray & body,
                              int timeoutMs,
                              bool logRequest);

    // Releases live socket state before thread shutdown.
    void shutdown();

private:
    bool ensureConnectedLocked(int timeoutMs);
    void closeSocketLocked();
    QString buildRequestTarget(const QString & path,
                               const QVector<QString> & queryParams) const;
    bool readResponseLocked(int timeoutMs, int &statusCode,
                            QHash<QByteArray, QByteArray> & responseHeaders,
                            QByteArray & responseBody);

private:
    Q_DISABLE_COPY(HttpClientSyncImpl)
    QSslSocket * socket = nullptr;
    QString baseHost;
    quint16 basePort = 0;
    bool baseTls = false;
};

// Owns a dedicated request thread and dispatches calls to HttpClientSyncImpl via invokeMethod.
class HttpClient {
public:
    explicit HttpClient(const QString & baseUrl);
    ~HttpClient();

    // Sync call from any thread. Work executes in the HttpClient thread.
    QString sendRequest(HTTP_CALL call, const QString & path,
                        const QVector<QString> & queryParams, // key/value
                        const QVector<QString> & headers, // key/value
                        const QByteArray & body,
                        int timeoutMs,
                        bool logRequest);

private:
    Q_DISABLE_COPY(HttpClient)

    QThread * requestThread = nullptr;
    HttpClientSyncImpl * httpClient = nullptr;
};

}

#endif //MWC_QT_WALLET_HTTPCLIENT_H

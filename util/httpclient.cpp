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
#include <QElapsedTimer>
#include <QHash>
#include <QMetaObject>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QThread>
#include <QUrl>
#include <QUrlQuery>

#include "stringutils.h"
#include "Log.h"

namespace util {

static bool parseStatusAndHeaders(const QByteArray &headerBlock,
                           int &statusCode,
                           QHash<QByteArray, QByteArray> &responseHeaders) {
    responseHeaders.clear();

    const QList<QByteArray> lines = headerBlock.split('\n');
    if (lines.isEmpty()) {
        return false;
    }

    const QList<QByteArray> rawStatusParts = lines.front().trimmed().split(' ');
    QList<QByteArray> statusParts;
    for (const QByteArray &part : rawStatusParts) {
        if (!part.isEmpty()) {
            statusParts.push_back(part);
        }
    }
    if (statusParts.size() < 2) {
        return false;
    }

    bool ok = false;
    statusCode = statusParts[1].toInt(&ok);
    if (!ok) {
        return false;
    }

    for (int i = 1; i < lines.size(); ++i) {
        const QByteArray line = lines[i].trimmed();
        if (line.isEmpty()) {
            continue;
        }
        const int idx = line.indexOf(':');
        if (idx <= 0) {
            continue;
        }

        const QByteArray key = line.left(idx).trimmed().toLower();
        const QByteArray value = line.mid(idx + 1).trimmed();
        responseHeaders.insert(key, value);
    }

    return true;
}

static bool decodeChunkedBody(const QByteArray &chunkedBody, QByteArray &decodedBody) {
    decodedBody.clear();

    int pos = 0;
    while (true) {
        const int chunkLenLineEnd = chunkedBody.indexOf("\r\n", pos);
        if (chunkLenLineEnd < 0) {
            return false;
        }

        QByteArray chunkLenLine = chunkedBody.mid(pos, chunkLenLineEnd - pos).trimmed();
        const int extPos = chunkLenLine.indexOf(';');
        if (extPos >= 0) {
            chunkLenLine = chunkLenLine.left(extPos);
        }

        bool ok = false;
        const int chunkLen = chunkLenLine.toInt(&ok, 16);
        if (!ok || chunkLen < 0) {
            return false;
        }

        pos = chunkLenLineEnd + 2;

        if (chunkLen == 0) {
            // End marker and optional trailers are expected to end with CRLFCRLF.
            const int trailersEnd = chunkedBody.indexOf("\r\n\r\n", pos);
            if (trailersEnd >= 0) {
                return true;
            }
            if (chunkedBody.size() >= pos + 2 && chunkedBody.mid(pos, 2) == "\r\n") {
                return true;
            }
            return false;
        }

        if (chunkedBody.size() < pos + chunkLen + 2) {
            return false;
        }

        decodedBody += chunkedBody.mid(pos, chunkLen);
        pos += chunkLen;

        if (chunkedBody.mid(pos, 2) != "\r\n") {
            return false;
        }
        pos += 2;
    }
}

HttpClientSyncImpl::HttpClientSyncImpl(const QString &baseUrl_, QObject * parent) : QObject(parent) {
    QUrl baseUrl(baseUrl_);
    baseHost = baseUrl.host();
    basePort = baseUrl.port(baseUrl.scheme().compare("https", Qt::CaseInsensitive) == 0 ? 443 : 80);
    baseTls = baseUrl.scheme().compare("https", Qt::CaseInsensitive) == 0;

    Q_ASSERT( !baseHost.isEmpty() && basePort>0 && basePort<65535 );
}

HttpClientSyncImpl::~HttpClientSyncImpl() {
    Q_ASSERT(socket==nullptr); //  closeSocketLocked() expected to be called before
}

bool HttpClientSyncImpl::ensureConnectedLocked(int timeoutMs) {
   Q_ASSERT(QThread::currentThread() == thread());

    if (baseHost.isEmpty() || basePort == 0) {
        Q_ASSERT(false);
        return false;
    }

    if (socket != nullptr && socket->state() == QAbstractSocket::ConnectedState)
        return true;

    closeSocketLocked();

    socket = new QSslSocket(this);
    socket->setPeerVerifyMode(QSslSocket::VerifyNone);
    socket->setProtocol(QSsl::TlsV1_2OrLater);

    if (baseTls) {
        socket->connectToHostEncrypted(baseHost, basePort);
        if (!socket->waitForEncrypted(timeoutMs)) {
            closeSocketLocked();
            return false;
        }
    } else {
        socket->connectToHost(baseHost, basePort);
        if (!socket->waitForConnected(timeoutMs)) {
            closeSocketLocked();
            return false;
        }
    }

    return true;
}

void HttpClientSyncImpl::closeSocketLocked() {
    Q_ASSERT(QThread::currentThread() == thread());

    if (socket == nullptr) {
        return;
    }

    socket->abort();
    delete socket;
    socket = nullptr;
}

QString HttpClientSyncImpl::buildRequestTarget(const QString &path,
                                       const QVector<QString> &queryParams) const {
    if (path.isEmpty()) {
        return QString();
    }

    QString requestPath = path.trimmed();

    if (!requestPath.startsWith('/')) {
        requestPath.prepend('/');
    }

    if (requestPath.isEmpty()) {
        requestPath = "/";
    }

    QUrlQuery query;
    Q_ASSERT(queryParams.size() % 2 == 0);
    for (int t = 1; t < queryParams.size(); t += 2) {
        query.addQueryItem(util::urlEncode(queryParams[t - 1]), util::urlEncode(queryParams[t]));
    }

    QString target = requestPath;
    const QString queryStr = query.query(QUrl::PrettyDecoded);
    if (!queryStr.isEmpty()) {
        target += "?" + queryStr;
    }

    return target;
}

bool HttpClientSyncImpl::readResponseLocked(int timeoutMs, int &statusCode,
                                    QHash<QByteArray, QByteArray> &responseHeaders,
                                    QByteArray &responseBody) {
    Q_ASSERT(QThread::currentThread() == thread());

    statusCode = 0;
    responseHeaders.clear();
    responseBody.clear();

    QElapsedTimer timer;
    timer.start();

    QByteArray buffer;
    int headerEnd = -1;
    bool chunkedEncoding = false;
    qint64 contentLength = -1;
    bool noBody = false;

    while (true) {
        if (socket == nullptr) {
            return false;
        }

        if (socket->bytesAvailable() == 0) {
            const int leftMs = timeoutMs - static_cast<int>(timer.elapsed());
            if (leftMs <= 0) {
                return false;
            }
            if (!socket->waitForReadyRead(leftMs)) {
                if (socket->state() == QAbstractSocket::UnconnectedState && headerEnd >= 0) {
                    // Server closed connection and body size is not specified.
                    responseBody = buffer.mid(headerEnd + 4);
                    return true;
                }
                return false;
            }
        }

        buffer += socket->readAll();

        if (headerEnd < 0) {
            headerEnd = buffer.indexOf("\r\n\r\n");
            if (headerEnd < 0) {
                continue;
            }

            const QByteArray headerBlock = buffer.left(headerEnd);
            if (!parseStatusAndHeaders(headerBlock, statusCode, responseHeaders)) {
                return false;
            }

            const QByteArray teHeader = responseHeaders.value("transfer-encoding").toLower();
            chunkedEncoding = teHeader.contains("chunked");

            if (responseHeaders.contains("content-length")) {
                bool ok = false;
                contentLength = responseHeaders.value("content-length").toLongLong(&ok);
                if (!ok || contentLength < 0) {
                    contentLength = -1;
                }
            }

            noBody = (statusCode >= 100 && statusCode < 200) || statusCode == 204 || statusCode == 304;
            if (noBody) {
                responseBody.clear();
                return true;
            }
        }

        const QByteArray payload = buffer.mid(headerEnd + 4);

        if (chunkedEncoding) {
            QByteArray decodedBody;
            if (decodeChunkedBody(payload, decodedBody)) {
                responseBody = decodedBody;
                return true;
            }
            continue;
        }

        if (contentLength >= 0) {
            if (payload.size() >= contentLength) {
                responseBody = payload.left(contentLength);
                return true;
            }
            continue;
        }

        if (socket->state() == QAbstractSocket::UnconnectedState) {
            responseBody = payload;
            return true;
        }
    }
}

QString HttpClientSyncImpl::sendRequest(HTTP_CALL call, const QString & path,
                          const QVector<QString> & queryParams, // key/value
                          const QVector<QString> & headers, // key/value
                          const QByteArray & body,
                          int timeoutMs,
                          bool logRequest) {
    Q_ASSERT(QThread::currentThread() == thread());

    qDebug() << "Sending request to base URL:" << baseHost << ":" << basePort
             << ", path:" << path << ", params: " << queryParams << ",  headers: " << headers;

    if (logRequest) {
        logger::logInfo(logger::HTTP_CLIENT, "Requesting: " + baseHost + ":" + QString::number(basePort) + path +
                        "   with body: " + util::string2shortStrR(QString::fromUtf8(body), 50));
    }

    if (call != POST) {
        Q_ASSERT(false);
        return "";
    }

    const QString target = buildRequestTarget(path, queryParams);
    if (target.isEmpty()) {
        Q_ASSERT(false);
        return "";
    }

    const bool defaultPort = (baseTls && basePort == 443) || (!baseTls && basePort == 80);

    QByteArray hostHeaderValue = baseHost.toUtf8();
    if (!defaultPort) {
        hostHeaderValue += ":" + QByteArray::number(basePort);
    }

    QByteArray requestData;
    requestData.reserve(body.size() + 1024);
    requestData += "POST " + target.toUtf8() + " HTTP/1.1\r\n";

    Q_ASSERT(headers.size() % 2 == 0);
    for (int t = 1; t < headers.size(); t += 2) {
        QByteArray headerName = headers[t - 1].trimmed().toUtf8();
        const QByteArray headerValue = headers[t].toUtf8();

        if (headerName.isEmpty()) {
            continue;
        }

        requestData += headerName + ": " + headerValue + "\r\n";
    }

    requestData += "Host: " + hostHeaderValue + "\r\n";
    requestData += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
    requestData += "Connection: keep-alive\r\n";

    requestData += "\r\n";
    requestData += body;

    QString response;
    for (int attempt = 0; attempt < 2; ++attempt) {
        if (!ensureConnectedLocked(timeoutMs)) {
            continue;
        }

        if (socket == nullptr) {
            continue;
        }

        if (socket->write(requestData) != requestData.size()) {
            closeSocketLocked();
            continue;
        }

        if (!socket->waitForBytesWritten(timeoutMs)) {
            closeSocketLocked();
            continue;
        }

        int statusCode = 0;
        QHash<QByteArray, QByteArray> responseHeaders;
        QByteArray responseBody;
        if (!readResponseLocked(timeoutMs, statusCode, responseHeaders, responseBody)) {
            closeSocketLocked();
            continue;
        }

        const QByteArray connectionHeader = responseHeaders.value("connection").toLower();
        if (connectionHeader.contains("close") || (socket != nullptr && socket->state() != QAbstractSocket::ConnectedState)) {
            closeSocketLocked();
        }

        if (statusCode < 200 || statusCode >= 300) {
            logger::logError(logger::HTTP_CLIENT,
                             "HTTP request to " + baseHost + target +
                             " failed with status " + QString::number(statusCode));
            return "";
        }

        response = QString::fromUtf8(responseBody).trimmed();
        break;
    }

    if (response.isEmpty()) {
        logger::logError(logger::HTTP_CLIENT, "Requesting to " + baseHost + target + "  is failed");
    }
    return response;
}

void HttpClientSyncImpl::shutdown() {
    Q_ASSERT(QThread::currentThread() == thread());
    closeSocketLocked();
}

HttpClient::HttpClient(const QString & baseUrl) {
    requestThread = new QThread();
    requestThread->setObjectName("HttpClient_" + baseUrl);
    requestThread->start();

    httpClient = new HttpClientSyncImpl(baseUrl);
    httpClient->moveToThread(requestThread);
}

HttpClient::~HttpClient() {
    HttpClientSyncImpl *client = httpClient;
    QMetaObject::invokeMethod(
            client,
            [client]() { client->shutdown(); },
            Qt::BlockingQueuedConnection);

    delete httpClient;
    httpClient = nullptr;

    requestThread->quit();
    requestThread->wait();
    requestThread = nullptr;
}

QString HttpClient::sendRequest(HTTP_CALL call, const QString &path,
                                    const QVector<QString> &queryParams, // key/value
                                    const QVector<QString> &headers, // key/value
                                    const QByteArray &body, int timeoutMs, bool logRequest) {
    HttpClientSyncImpl *client = httpClient;
    QString response;
    const bool invoked = QMetaObject::invokeMethod(
        client,
        [&response, client, call, path, queryParams, headers, body, timeoutMs, logRequest]() {
            response = client->sendRequest(call, path, queryParams, headers, body, timeoutMs, logRequest);
        },
        Qt::BlockingQueuedConnection);

    if (!invoked) {
        logger::logError(logger::HTTP_CLIENT, "Failed to dispatch request to HttpClient thread");
        return "";
    }

    return response;
}

}

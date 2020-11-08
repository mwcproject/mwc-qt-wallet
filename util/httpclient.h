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

// Http calls funtionality for the any class
class HttpClient : public QObject {
Q_OBJECT
protected:
    enum HTTP_CALL {
        GET, POST
    };

    HttpClient();

    void sendRequest(HTTP_CALL call, const QString &url,
                     const QString &tag,
                     const QVector<QString> &params,
                     const QByteArray &body = "", //
                     const QString &param1="", const QString &param2="",
                     const QString &param3="", const QString &param4="");

    virtual void onProcessHttpResponse(bool requestOk, const QString & tag, QJsonObject & jsonRespond,
                                       const QString & param1,
                                       const QString & param2,
                                       const QString & param3,
                                       const QString & param4) = 0;

private slots:
    void replyFinished(QNetworkReply* reply);

protected:
    QNetworkAccessManager * nwManager = nullptr;
};

}

#endif //MWC_QT_WALLET_HTTPCLIENT_H

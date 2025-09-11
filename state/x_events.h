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

#ifndef EVENTS_H
#define EVENTS_H

#include "state.h"
#include "../core/Notification.h"
#include <QVector>
#include <QMap>
#include <QSet>

#include "wallet/wallet.h"


namespace state {

struct ReceivedTxInfo {
    QString mwc;
    QString fromAddr;
    QString message;

    ReceivedTxInfo() = default;
    ReceivedTxInfo(const ReceivedTxInfo &) = default;
    ReceivedTxInfo(const QString &_mwc, const QString &_fromAddr, const QString &_message)
        : mwc(_mwc),
          fromAddr(_fromAddr),
          message(_message) {
    }

    ReceivedTxInfo& operator=(const ReceivedTxInfo&) = default;
};

class Events : public QObject, public State
{
    Q_OBJECT
public:
    Events( StateContext * context );
    virtual ~Events() override;

    void eventsWndIsDeleted();

    QVector<notify::NotificationMessage> getWalletNotificationMessages();

    // Check if some error/warnings need to be shown
    bool hasNonShownWarnings() const;
public:

private slots:
    void onNewNotificationMessage(bridge::MESSAGE_LEVEL level, QString message);

    void onSlateReceivedFrom(QString slate, QString mwc, QString fromAddr, QString message );
    void onTransactionById( bool success, QString account, int64_t height, wallet::WalletTransaction transaction,
                                QVector<wallet::WalletOutput> outputs, QVector<QString> messages );

protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override {return false;}
    virtual QString getHelpDocName() override {return "event_log.html";}
private:
    int64_t         messageWaterMark = 0;
    // Transactions that we are processing for congrats request
    QMap<QString, ReceivedTxInfo> recievedTxs;
    QSet<QString>   activeUUID;

};

}

#endif // EVENTS_H

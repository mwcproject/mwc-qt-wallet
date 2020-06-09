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

#ifndef MWC_QT_WALLET_X_EVENTS_B_H
#define MWC_QT_WALLET_X_EVENTS_B_H

#include <QObject>
#include <QVector>

namespace bridge {

class Events : public QObject {
    Q_OBJECT
public:
    explicit Events(QObject * parent = nullptr);
    ~Events();

public:
    // New message arrive, need to emit the signal about that
    void updateShowMessages();
    // Update status that New Warning message has arrived
    void updateNonShownWarnings(bool hasNonShownWarns);

public:
    // Request is any new critical warnings arrived
    Q_INVOKABLE bool hasNonShownWarnings();

    // Notify thet events window is closed. Last seen even waterwart will be updated
    Q_INVOKABLE void eventsWndIsDeleted();

    // Request notofication messages to show.
    // Return groups of 5: [time short, time long, level short, level full, message]
    Q_INVOKABLE QVector<QString> getWalletNotificationMessages();


signals:
    void sgnUpdateShowMessages();
    void sgnUpdateNonShownWarnings(bool hasNonShownWarns);
};

}

#endif //MWC_QT_WALLET_X_EVENTS_B_H

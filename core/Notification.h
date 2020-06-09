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


#ifndef MWC_QT_WALLET_NOTIFICATION_H
#define MWC_QT_WALLET_NOTIFICATION_H

#include <QObject>
#include <QDateTime>

namespace notify {

// Report Message to the wallet.
enum class MESSAGE_LEVEL { FATAL_ERROR=1, CRITICAL=2, WARNING=3, INFO=4, DEBUG=5 };

// Enum to string
QString toString(MESSAGE_LEVEL level);

// Some notifications can have different meaning because of the state. For example when we stop/start node, mwc713 error connection messages are expected.
enum NOTIFICATION_STATES { ONLINE_NODE_IMPORT_EXPORT_DATA = 0x0001 };
void notificationStateSet(NOTIFICATION_STATES state);
void notificationStateClean(NOTIFICATION_STATES state);
bool notificationStateCheck(NOTIFICATION_STATES state);

void addFalseMessage(const QString & msg);
void remeoveFalseMessage(const QString & msg);

struct NotificationMessage {
    MESSAGE_LEVEL level = MESSAGE_LEVEL::DEBUG;
    QString message;
    QDateTime time;

    NotificationMessage() {time=QDateTime::currentDateTime();}
    NotificationMessage(MESSAGE_LEVEL _level, QString _message) : level(_level), message(_message) { time = QDateTime::currentDateTime(); }
    NotificationMessage(const NotificationMessage&) = default;
    NotificationMessage &operator=(const NotificationMessage&) = default;

    QString getLevelStr() const;
    QString getLevelLongStr() const;

    static bool isCritical(MESSAGE_LEVEL l) {return l<=MESSAGE_LEVEL::WARNING;}
    bool isCritical() const {return isCritical(level);}

    // To debug string
    QString toString() const;
};

// Special object for messages
class Notification : public QObject {
Q_OBJECT

public:
    // Needed to listen for notification, use this instance for that...
    static Notification * getObject2Notify();

    void sendNewNotificationMessage(MESSAGE_LEVEL level, QString message);

private:
    Notification();

private:
signals:
    // Notification/error message
    void onNewNotificationMessage(MESSAGE_LEVEL level, QString message);

};

// Generic. Reporting fatal error that somebody will process and exit app
void reportFatalError( QString message );

// Get all notification messages
// Check signal: Notification::onNewNotificationMessage
QVector<NotificationMessage> getNotificationMessages();


void appendNotificationMessage( MESSAGE_LEVEL level, QString message );



}

Q_DECLARE_METATYPE(notify::MESSAGE_LEVEL );


#endif //MWC_QT_WALLET_NOTIFICATION_H

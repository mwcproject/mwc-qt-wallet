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


#include "Notification.h"
#include "../core/global.h"
#include "../util/Log.h"
#include <QSet>
#include <QVector>
#include "WndManager.h"
#include "MessageMapper.h"
#include "../bridge/notification_b.h"

namespace notify {

static uint notificationState = 0;
void notificationStateSet(NOTIFICATION_STATES state) {
    notificationState |= state;
}

void notificationStateClean(NOTIFICATION_STATES state) {
    notificationState &=  ~uint(state);
}

bool notificationStateCheck(NOTIFICATION_STATES state) {
    return notificationState & state ? true : false;
}

// Message that will be requlified from Critical to Info
static QSet<QString> falseCriticalMessages{"keybase not found! consider installing keybase locally first."};
static QSet<QString> falseMessages;

void addFalseMessage(const QString & msg) {
    falseMessages.insert(msg);
}
void remeoveFalseMessage(const QString & msg) {
    falseMessages.remove(msg);
}

const int MESSAGE_SIZE_LIMIT = 1000;
static QVector<NotificationMessage> notificationMessages;

// Enum to string
QString toString(bridge::MESSAGE_LEVEL level) {
    switch (level) {
        case bridge::MESSAGE_LEVEL::FATAL_ERROR:
            return "FATAL ERROR";
        case bridge::MESSAGE_LEVEL::CRITICAL:
            return "CRITICAL";
        case bridge::MESSAGE_LEVEL::WARNING:
            return "WARNING";
        case bridge::MESSAGE_LEVEL::INFO:
            return "INFO";
        case bridge::MESSAGE_LEVEL::DEBUG:
            return "DEBUG";
    }
    return "UNKNOWN";
}


////////////////////////////////////////////////////////////////////////////////////
//      NotificationMessage

QString NotificationMessage::getLevelStr() const {
    switch (level) {
        case bridge::MESSAGE_LEVEL::FATAL_ERROR: return "Err";
        case bridge::MESSAGE_LEVEL::CRITICAL:  return "Crit";
        case bridge::MESSAGE_LEVEL::WARNING:   return "Warn";
        case bridge::MESSAGE_LEVEL::INFO:      return "info";
        case bridge::MESSAGE_LEVEL::DEBUG:     return "dbg";
        default:   Q_ASSERT(false); return "???";
    }
}

QString NotificationMessage::getLevelLongStr() const {
    switch (level) {
        case bridge::MESSAGE_LEVEL::FATAL_ERROR: return "Error";
        case bridge::MESSAGE_LEVEL::CRITICAL:  return "Critical";
        case bridge::MESSAGE_LEVEL::WARNING:   return "Warning";
        case bridge::MESSAGE_LEVEL::INFO:      return "Info";
        case bridge::MESSAGE_LEVEL::DEBUG:     return "Debug";
        default:   Q_ASSERT(false); return "???";
    }
}

// To debug string
QString NotificationMessage::toString() const {
    return ( "NotifMsg(level=" + getLevelStr() + ", message="+message + ")" );
}

////////////////////////////////////////////////////////////////////////////////////
//      Notification

Notification::Notification() {}

static Notification * singletineNotification = nullptr;

Notification * Notification::getObject2Notify() {
    if (singletineNotification == nullptr) {
        singletineNotification = new Notification();
    }
    return singletineNotification;
}

void Notification::sendNewNotificationMessage(bridge::MESSAGE_LEVEL level, QString message) {
    emit onNewNotificationMessage(level, message);
}


// Get all notification messages
// Check signal: Notification::onNewNotificationMessage
QVector<NotificationMessage> getNotificationMessages() {
    return notificationMessages;
}

// Generic. Reporting fatal error that somebody will process and exit app
void reportFatalError( QString message )  {
    appendNotificationMessage( bridge::MESSAGE_LEVEL::FATAL_ERROR, message );
}

void appendNotificationMessage( bridge::MESSAGE_LEVEL level, QString message ) {
    if (message.isEmpty())
        return;

    logger::logInfo("Notification", toString(level) + "  " + message );

    for (const auto & fm : falseMessages) {
        if ( message.startsWith(fm) )
            return;
    }

    static MessageMapper msgMapper(":/resource/notification_mappers.txt");

    QString m = msgMapper.processMessage(message);
    if (m != message) {
        logger::logInfo("Notification", "Message is updated: " + m );
        message = m;
    }

    if (message.isEmpty() || message=="Swap trade not found")
        return; // Ignoring

    if (level == bridge::MESSAGE_LEVEL::FATAL_ERROR) {
        // Fatal error. Display message box and exiting. We don't want to continue
        core::getWndManager()->messageTextDlg("Wallet Error", "Wallet got a critical error:\n" + message + "\n\nPress OK to exit the wallet" );
        mwc::closeApplication();
        return;
    }

    // Message is not fatal, adding it into the logs
    if (level == bridge::MESSAGE_LEVEL::CRITICAL) {
        if ( falseCriticalMessages.contains(message) )
            level = bridge::MESSAGE_LEVEL::INFO;
    }

    NotificationMessage msg(level, message);

    // check if it is duplicate message. Duplicates will be ignored.
    if (! ( notificationMessages.size()>0 && notificationMessages.last().message == message ) ) {
        notificationMessages.push_back(msg);

        while (notificationMessages.size() > MESSAGE_SIZE_LIMIT)
            notificationMessages.pop_front();
    }

    logger::logEmit( "MWC713", "onNewNotificationMessage", msg.toString() );

    Notification::getObject2Notify()->sendNewNotificationMessage(msg.level, msg.message);
}

}

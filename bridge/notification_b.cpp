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

#include "notification_b.h"
#include "../core/Notification.h"
#include "../util/Log.h"

namespace bridge {

Notification::Notification(QObject * parent) : QObject(parent) {
    QObject::connect( notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
                  this, &Notification::onSgnNewNotificationMessage, Qt::QueuedConnection);

}
Notification::~Notification() {}

void Notification::appendNotificationMessage(MESSAGE_LEVEL level, QString message) {
    logger::logInfo(logger::BRIDGE, "Call Notification::appendNotificationMessage with level=" + QString::number(static_cast<int>(level)) + " message=" + message);
    notify::appendNotificationMessage( level, message );
}

void Notification::onSgnNewNotificationMessage(bridge::MESSAGE_LEVEL level, QString message) // level: bridge::MESSAGE_LEVEL
{
    emit sgnNewNotificationMessage( int(level), message);
}

}

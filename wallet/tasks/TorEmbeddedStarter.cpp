// Copyright 2025 The MWC Developers
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

#include "TorEmbeddedStarter.h"

#include "core/Config.h"
#include "core/Notification.h"
#include "node/TorProcess.h"
#include "util/ioutils.h"
#include "util/message_mapper.h"
#include <QtConcurrent>

namespace wallet {

QFuture<QString> startEmbeddedTor() {
    return QtConcurrent::run([] () -> QString {
            QThread::currentThread()->setObjectName("startEmbeddedTor");
            if ( !config::isColdWallet() ) {
                // Starting tor core
                QPair<bool,QString> path = ioutils::getAppDataPath("tor");
                if (!path.first) {
                    notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::CRITICAL, "Unable to prepare Tor data directory. " + util::mapMessage(path.second));
                    return path.second;
                }

                QString startRes = tor::startEmbeddedTor(path.second);
                if (!startRes.isEmpty()) {
                    notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::CRITICAL, "Unable to start embedded Tor service. "+ util::mapMessage(startRes));
                    return "Unable to start embedded Tor service.\n"+ startRes;
                }
            }
            QThread::currentThread()->setObjectName("QTThreadPool");
            return "";
    });
}

}

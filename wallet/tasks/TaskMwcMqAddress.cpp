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

#include "TaskMwcMqAddress.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../core/Notification.h"

namespace wallet {

bool TaskMwcMqAddress::processTask(const QVector<WEvent> & events) {

    qDebug() << "TaskMwcMqAddress::processTask: " << printEvents(events);

    QVector< WEvent > mwcMqAddr = filterEvents(events, WALLET_EVENTS::S_YOUR_MWC_ADDRESS );
    QVector< WEvent > mwcMqAddrIdx = filterEvents(events, WALLET_EVENTS::S_MWC_ADDRESS_INDEX );

    if (mwcMqAddr.empty()) {
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL,
                                             "Unable to get the MWC MQS address");
        return true;
    }

    QString address = mwcMqAddr[0].message;

    int idx=-1;

    if (mwcMqAddrIdx.size()>0) {
        bool ok = false;
        idx = mwcMqAddrIdx[0].message.toInt(&ok);
        if (!ok)
            idx=-1;
    }

    qDebug() << "Get MWC MQS address " << address << " for index " << idx;

    if (idx>=0)
        wallet713->setMwcAddressWithIndex( address, idx );

    return true;
}

QString TaskMwcMqAddress::calcCommandLine( bool genNext, int idx ) const {
    if (genNext) {
        if (idx>=0)
            return "config --generate-next-address -i " + QString::number(idx);
        else
            return "config --generate-next-address";
    }
    else
        return "address";
}


}



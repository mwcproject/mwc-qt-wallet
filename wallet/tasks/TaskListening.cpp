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

#include "TaskListening.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../core/Config.h"

namespace wallet {

// ------------------------------- TaskListeningListener ------------------------------------------

bool TaskListeningListener::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];

    switch (evt.event) {
        case S_YOUR_MWC_ADDRESS: {
            QString address = evt.message;
            if (address.length()==0) {
                wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::WARNING, MWC713::MESSAGE_ID::GENERIC,
                                                      "mwc713 responded with empty MWC address" );
            }
            wallet713->setMwcAddress(address);
            return true;
        }
        case S_LISTENER_ON: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);

            if ( evt.message == "keybase" ) {
                wallet713->setKeybaseListeningStatus(true);
            }
            else {
                const QString & addrees = evt.message;
                // x prefix is for testnet
                // q - for mainnet
                if (addrees.size()>0 && (addrees[0]=='x' || addrees[0]=='q') ) {
                    wallet713->setMwcMqListeningStatus(true);
                    wallet713->setMwcAddress(evt.message);
                }
            }
            return true;
        }
        case S_LISTENER_OFF: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            if ( evt.message == "keybase" ) {
                wallet713->setKeybaseListeningStatus(false);
            }
            else {
                wallet713->setMwcMqListeningStatus(false);
            }
            return true;
        }
        case S_LISTENER_MQ_LOST_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setMwcMqListeningStatus(false);
            return true;
        }
        case S_LISTENER_MQ_GET_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setMwcMqListeningStatus(true);
            return true;
        }
        case S_LISTENER_MQ_COLLISION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->notifyListenerMqCollision();
            return true;
        }
        default:
            return false;
    }
}

// ----------------------------------- TaskListeningStart --------------------------------------

bool TaskListeningStart::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskListeningStart::processTask with events: " << printEvents(events);

    QVector< WEvent > mqStaring = filterEvents(events, WALLET_EVENTS::S_LISTENER_MQ_STARTING );
    QVector< WEvent > kbStaring = filterEvents(events, WALLET_EVENTS::S_LISTENER_KB_STARTING );

    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_ERROR );

    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    wallet713->setListeningStartResults( mqStaring.size()>0, kbStaring.size()>0, // what we try to start
                                         errorMessages );

    return true;
}

QString TaskListeningStart::calcCommand(bool startMq, bool startKeybase) const {
    Q_ASSERT(startMq | startKeybase);

    // -m, --mwcmq      mwcmq listener
    // -k, --keybase    keybase listener
    // -s, --mwcmqs     mwcmqs listener
    static QString mq2start = config::getUseMwcMqS() ? " -s" : " -m";

    return QString("listen") + (startMq ? mq2start : "") + (startKeybase ? " -k" : "");
}

// -------------------------------- TaskListeningStop -------------------------------

bool TaskListeningStop::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskListeningStop::processTask with events: " << printEvents(events);

    QVector< WEvent > mqStopping = filterEvents(events, WALLET_EVENTS::S_LISTENER_MQ_STOPPING );
    QVector< WEvent > kbStopping = filterEvents(events, WALLET_EVENTS::S_LISTENER_KB_STOPPING );

    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_ERROR );

    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    qDebug() << "results: mq=" << mqStopping.size() << ",kb=" << kbStopping.size();
    wallet713->setListeningStopResult( mqStopping.size()>0, kbStopping.size()>0,
                                       errorMessages );

    return true;
}

QString TaskListeningStop::calcCommand(bool stopMq, bool stopKeybase) const {
    Q_ASSERT(stopMq | stopKeybase);

    // -m, --mwcmq      mwcmq listener
    // -k, --keybase    keybase listener
    // -s, --mwcmqs     mwcmqs listener
    static QString mq2stop = config::getUseMwcMqS() ? " -s" : " -m";

    return QString("stop") + (stopMq ? mq2stop : "") + (stopKeybase ? " -k" : "");
}


}



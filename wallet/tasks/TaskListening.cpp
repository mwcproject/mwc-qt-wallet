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
#include "../../core/Notification.h"

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
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING,
                                                      "mwc713 responded with empty MWC address" );
            }
            wallet713->setMwcAddress(address);
            return true;
        }
        case S_LISTENER_ON: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);

            QStringList prms = evt.message.split('|');
            if ( prms.size()==0 )
                return false;

            if ( prms[0] == "keybase" ) {
                wallet713->setKeybaseListeningStatus(true);
            }
            else {
                const QString & address = prms[0];
                // x prefix is for testnet
                // q - for mainnet
                if (address.size()>0 && (address[0]=='x' || address[0]=='q') ) {
                    wallet713->setMwcMqListeningStatus(true, prms.size()>1 ? prms[1] : "", true);
                    wallet713->setMwcAddress(prms[0] );
                // last case for Tor it will be http://something.onion
                }
                else if (address.size()>0 && (address[0]=='h') ) {
                    wallet713->setTorListeningStatus(true);
                    wallet713->setTorAddress(prms[0] );
                }
            }
            return true;
        }
        case S_LISTENER_OFF: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);

            QStringList prms = evt.message.split('|');
            if ( prms.size()==0 ) {
                // It is tor
                wallet713->setTorListeningStatus(false);
            }
            if ( prms[0] == "keybase" ) {
                wallet713->setKeybaseListeningStatus(false);
            }
            else {
                wallet713->setMwcMqListeningStatus(false, prms.size()>1 ? prms[1] : "" , true);
            }
            return true;
        }
        case S_LISTENER_MQ_LOST_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            QStringList prms = evt.message.split('|');
            wallet713->setMwcMqListeningStatus(false, prms.size()>1 ? prms[1] : "", false );
            return true;
        }
        case S_LISTENER_MQ_GET_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            QStringList prms = evt.message.split('|');
            wallet713->setMwcMqListeningStatus(true, prms.size()>1 ? prms[1] : "", false );
            return true;
        }
        case S_LISTENER_KB_LOST_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setKeybaseListeningStatus(false);
            return true;
        }
        case S_LISTENER_KB_GET_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setKeybaseListeningStatus(true);
            return true;
        }
        case S_LISTENER_TOR_LOST_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setTorListeningStatus(false);
            return true;
        }
        case S_LISTENER_TOR_GET_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setTorListeningStatus(true);
            return true;
        }
        case S_LISTENER_MQ_COLLISION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->notifyListenerMqCollision();
            return true;
        }
        case S_LISTENER_MQ_FAILED_TO_START: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->notifyMqFailedToStart();
            return true;
        }
        case S_LISTENER_HTTP_STARTING: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            QString address = evt.message;
            wallet713->setHttpListeningStatus(true, address);
            return true;
        }
        case S_LISTENER_HTTP_FAILED: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            QString error = evt.message;
            wallet713->setHttpListeningStatus(false, error);
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING,
                                               "mwc713 Unable to start Http Foreign API. Error: " + error );
            return true;
        }

        default:
            return false;
    }
}

// ----------------------------------- TaskListeningStart --------------------------------------

bool TaskListeningStart::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskListeningStart::processTask with events: " << printEvents(events);

    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR );
    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    bool mqs = false;
    bool keybase = false;
    bool tor = false;
    for (const WEvent & l : filterEvents(events, WALLET_EVENTS::S_LINE ) ) {
        if (l.message.contains("Starting mwcmqs listener"))
            mqs = true;
        if (l.message.contains("Starting keybase listener"))
            keybase = true;
        if (l.message.contains("Starting Tor listener"))
            tor = true;
    }

    wallet713->setListeningStartResults( mqs, keybase, tor, // what we try to start
                                         errorMessages, initialStart );

    return true;
}

QString TaskListeningStart::calcCommand(bool startMq, bool startKeybase, bool startTor) const {
    Q_ASSERT(startMq | startKeybase | startTor);

    // if tor, return listen -t
    if(startTor)
        return QString("listen -t");

    // -m, --mwcmq      mwcmq listener
    // -k, --keybase    keybase listener
    // -s, --mwcmqs     mwcmqs listener
    return QString("listen") + (startMq ? " -s" : "") + (startKeybase ? " -k" : "");
}

// -------------------------------- TaskListeningStop -------------------------------

bool TaskListeningStop::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskListeningStop::processTask with events: " << printEvents(events);

    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR );
    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    bool mqs = false;
    bool keybase = false;
    bool tor = false;
    for (const WEvent & l : filterEvents(events, WALLET_EVENTS::S_LINE ) ) {
        if (l.message.contains("Stopping mwcmqs listener"))
            mqs = true;
        if (l.message.contains("Stopping keybase listener"))
            keybase = true;
        if (l.message.contains("Stopping Tor listener", Qt::CaseInsensitive))
            tor = true;
    }

    wallet713->setListeningStopResult( mqs, keybase, tor,
                                       errorMessages );

    return true;
}

QString TaskListeningStop::calcCommand(bool stopMq, bool stopKeybase, bool stopTor) const {
    Q_ASSERT(stopMq | stopKeybase | stopTor);

    if(stopTor)
        return QString("stop -t");

    // -m, --mwcmq      mwcmq listener
    // -k, --keybase    keybase listener
    // -s, --mwcmqs     mwcmqs listener
    return QString("stop") + (stopMq ? " -s" : "") + (stopKeybase ? " -k" : "");
}


}



#include "TaskListeningListener.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

bool TaskListeningListener::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];

    switch (evt.event) {
        case S_LISTENER_MQ_ON: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setMwcMqListeningStatus(true);
            wallet713->setMwcAddress(evt.message);
            return true;
        }
        case S_LISTENER_KB_ON: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setKeybaseListeningStatus(true);
            return true;
        }
        case S_LISTENER_MQ_OFF: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setMwcMqListeningStatus(false);
            return true;
        }
        case S_LISTENER_KB_OFF: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setKeybaseListeningStatus(false);
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
        default:
            return false;
    }
}

}

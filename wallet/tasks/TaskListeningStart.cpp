#include "TaskListeningStart.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

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

    return QString("listen") + (startMq ? " -m" : "") + (startKeybase ? " -k" : "");
}


}
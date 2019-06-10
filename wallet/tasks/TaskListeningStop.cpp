#include "TaskListeningStop.h"

#include <QDebug>
#include "../mwc713.h"

namespace wallet {

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

    wallet713->setListeningStopResult( mqStopping.size()>0, kbStopping.size()>0,
              errorMessages );

    return true;
}

QString TaskListeningStop::calcCommand(bool stopMq, bool stopKeybase) const {
    Q_ASSERT(stopMq | stopKeybase);

    return QString("stop") + (stopMq ? " -m" : "") + (stopKeybase ? " -k" : "");
}


}
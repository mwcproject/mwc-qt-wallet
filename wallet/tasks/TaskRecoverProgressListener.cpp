#include "TaskRecoverProgressListener.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

bool TaskRecoverProgressListener::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];
    if (evt.event != S_RECOVERY_PROGRESS)
        return false;

    qDebug() << "TaskRecoverProgressListener::processTask with events: " << printEvents(events);

    QStringList lst = evt.message.split('|');
    Q_ASSERT(lst.size()==2);
    if (lst.size()!=2)
        return false;

    bool ok0 = false;
    long limit = lst[0].toLong(&ok0); // Limit
    bool ok1 = false;
    long pos = lst[1].toLong(&ok1); // position

    Q_ASSERT(ok0 && ok1);
    if (ok0 && ok1) {
        Q_ASSERT(pos<=limit);
        wallet713->setRecoveryProgress( pos, limit );
        return true;
    }

    return false;
}


}
#include "TaskRecover1Type.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

bool TaskRecover1Type::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskRecover1Type::processTask with events: " << printEvents(events);

    QVector< WEvent > mnem = filterEvents(events, WALLET_EVENTS::S_RECOVERY_MNEMONIC );

    if (mnem.isEmpty())
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC, "Get wrong respond from mwc713 process." );

    return true;
}

}
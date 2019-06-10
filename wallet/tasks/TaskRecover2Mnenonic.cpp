#include "TaskRecover2Mnenonic.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

bool TaskRecover2Mnenonic::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskRecover2Mnenonic::processTask with events: " << printEvents(events);

    QVector< WEvent > password = filterEvents(events, WALLET_EVENTS::S_PASSWORD_EXPECTED );
    if (password.isEmpty())
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC, "Get wrong respond from mwc713 process." );

    return true;
}

QString TaskRecover2Mnenonic::calcCommand(QVector<QString> seed) const {
    Q_ASSERT( seed.size() == 24 );

    QString cmd;
    for (auto & s : seed) {
        if (!cmd.isEmpty())
            cmd += " ";
        cmd += s;
    }
    return cmd;
}

}


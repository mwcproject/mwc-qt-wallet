#include "TaskInit.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

bool TaskInit::processTask(const QVector<WEvent> & events) {

    qDebug() << "TaskInit::processTask: " << printEvents(events);

    while (true) {
        // happy path

        QVector< WEvent > evt1 = filterEvents(events, WALLET_EVENTS::S_INIT_IN_PROGRESS );
        QVector< WEvent > evt2 = filterEvents(events, WALLET_EVENTS::S_PASSWORD_EXPECTED );

        if (evt1.empty() ||evt2.empty())
            break;

        // Ready to get a password. Expected that nexttask will take care about that
        return true;
    }

    // Failure path. Just report a error
    wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC,
                                         "Unable to init wallet and get a passphrase for you");
    return true;
}

}


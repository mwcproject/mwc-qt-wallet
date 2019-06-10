#include "TaskInitPassphrase.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {


bool TaskInitPassphrase::processTask(const QVector<WEvent> & events) {

    qDebug() << "TaskInitPassphrase::processTask: " << printEvents(events);

    while (true) {
        // happy path

        QVector< WEvent > passPhrase = filterEvents(events, WALLET_EVENTS::S_PASS_PHRASE );
        QVector< WEvent > evt2 = filterEvents(events, WALLET_EVENTS::S_INIT_WANT_ENTER );

        if (passPhrase.empty() ||evt2.empty())
            break;

        // Get a passphrase, let's parse it
        QStringList phr = passPhrase[0].message.split(" ");
        qDebug() << "Get a passphrase, it has words: " << phr.size();

        QVector<QString> seed;

        for (QString & s : phr)
            seed.push_back(s);

        wallet713->setNewSeed( seed );
        return true;
    }

    // Failure path. Just report a error
    wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC,
                                         "Unable to init wallet and get a passphrase for you");
    return true;
}

}


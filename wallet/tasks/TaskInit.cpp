#include "TaskInit.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../util/Log.h"

namespace wallet {

// ------------------------------ TaskInit ----------------------------

bool TaskInit::processTask(const QVector<WEvent> & events) {

    qDebug() << "TaskInit::processTask: " << printEvents(events);

    QVector< WEvent > evt2 = filterEvents(events, WALLET_EVENTS::S_PASSWORD_EXPECTED );
    if (evt2.empty()) {
        wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC,
                                             "Unable to init wallet and get a passphrase for you");
    }

    return true;
}

// ------------------------- TaskInitConfirm --------------------------

TaskInitConfirm::~TaskInitConfirm() {}


bool TaskInitConfirm::processTask(const QVector<WEvent> & events) {

    qDebug() << "TaskInitConfirm::processTask: " << printEvents(events);

    // Here we can't fail
    QVector< WEvent > mwcAddr = filterEvents(events, WALLET_EVENTS::S_YOUR_MWC_ADDRESS );

    if (mwcAddr.size()>0) {
        QString address = mwcAddr[0].message;
        if (address.length()==0) {
            wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::WARNING, MWC713::MESSAGE_ID::GENERIC,
                                                  "mwc713 responded with empty MWC address" );
        }
        else {
            wallet713->setMwcAddress(address);
        }
    }

    return true;
}

// ---------------------------------- TaskInitPassphrase -----------------------------------

void TaskInitPassphrase::onStarted() {
    logger::blockLogMwc713out( true );
}

bool TaskInitPassphrase::processTask(const QVector<WEvent> & events) {

    logger::blockLogMwc713out( false );

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


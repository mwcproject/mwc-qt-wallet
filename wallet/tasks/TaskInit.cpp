#include "TaskInit.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../util/Log.h"

namespace wallet {

QVector<QString> calcSeedFromEvents(const QVector<WEvent> &events) {
    QVector<WEvent> lines = filterEvents(events, WALLET_EVENTS::S_LINE);

    int idx = 0;

    for (; idx < lines.size(); idx++) {
        if (lines[idx].message.contains("Your recovery phrase is")) { // This line might be started with prompt...
            idx++;
            break;
        }
    }

    QString passPhs;

    // Expected 24 words. 23 spaces plus many letters
    for (; idx < lines.size() && passPhs.size() < 42; idx++) {
        passPhs = lines[idx].message;
    }

    bool found = false;
    for (; idx < lines.size(); idx++) {
        if (lines[idx].message.contains("Please back-up these words in a non-digital format")) {
            found = true;
            break;
        }
    }

    if (!found)
        return QVector<QString>();

    // Get a passphrase, let's parse it
    QStringList phr = passPhs.split(" ");
    qDebug() << "Get a passphrase, it has words: " << phr.size();

    QVector<QString> seed;

    for (QString &s : phr)
        seed.push_back(s);

    return seed;
}


void TaskInit::onStarted() {
   // logger::blockLogMwc713out( true );
}

bool TaskInit::processTask(const QVector<WEvent> & events) {

    logger::blockLogMwc713out(false);

    qDebug() << "TaskInit::processTask: " << printEvents(events);

    wallet713->setNewSeed(calcSeedFromEvents(events));
    return true;
}


}


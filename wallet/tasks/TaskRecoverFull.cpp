#include "TaskRecoverFull.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

bool ProcessRecoverTask(const QVector<WEvent> &events,MWC713 *wallet713) {
    qDebug() << "ProcessRecoverTask with events: " << printEvents(events);

    QVector< WEvent > staring = filterEvents(events, WALLET_EVENTS::S_RECOVERY_STARTING );
    QVector< WEvent > done = filterEvents(events, WALLET_EVENTS::S_RECOVERY_DONE );
    QVector< WEvent > mqAddr = filterEvents(events, WALLET_EVENTS::S_YOUR_MWC_ADDRESS );
    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_ERROR );

    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    wallet713->setRecoveryResults( staring.size() > 0, done.size()>0,
                                   mqAddr.size()<1 ? "" : mqAddr[0].message,
                                   errorMessages );
    return true;
}


bool TaskRecoverFull::processTask(const QVector<WEvent> &events) {
    return ProcessRecoverTask(events, wallet713);
}

QString TaskRecoverFull::calcCommand(QVector<QString> seed, QString password) const {
    Q_ASSERT( seed.size() == 24 );
    Q_ASSERT( password.size()>0 );

    QString cmd = "recover --mnemonic ";
    for (auto & s : seed)
        cmd += " " + s;

    cmd += " -p " + password;
    return cmd;
}

}


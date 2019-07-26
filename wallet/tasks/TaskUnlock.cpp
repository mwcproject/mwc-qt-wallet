#include "TaskUnlock.h"
#include "../mwc713.h"
#include <QDebug>

namespace wallet {

    bool TaskUnlock::processTask(const QVector<WEvent> & events) {
        qDebug() << "TaskUnlock::processTask with events: " << printEvents(events);

        QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_PASSWORD_ERROR );

        wallet713->setLoginResult( error.empty() );

        if (error.empty())
            wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::WARNING, MWC713::MESSAGE_ID::GENERIC,"Successfully logged into the wallet");
        return true;
    }

    // static
    QString TaskUnlock::buildWalletRequest(QString password) {
        QString res = "unlock";
        if (password.length() > 0)
            res += " -p " + util::toMwc713input(password);
        return res;
    }

}

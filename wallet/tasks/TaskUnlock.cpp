#include "TaskUnlock.h"
#include "../mwc713.h"
#include <QDebug>

namespace wallet {

    bool TaskUnlock::processTask(const QVector<WEvent> & events) {
        qDebug() << "TaskUnlock::processTask with events: " << printEvents(events);

        QVector< WEvent > mwcAddr = filterEvents(events, WALLET_EVENTS::S_YOUR_MWC_ADDRESS );
        QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_PASSWORD_ERROR );

        if (!error.empty()) {
            wallet713->setInitStatus( MWC713::INIT_STATUS::WRONG_PASSWORD );
            return true;
        }

        if (mwcAddr.empty()) {
            wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::WARNING, MWC713::MESSAGE_ID::GENERIC,
                    "Unlock didn't respond with mwc address. Something wrong with mwc713 wallet?" );
            return true;
        }

        QString address = mwcAddr[0].message;
        if (address.length()==0) {
            wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::WARNING, MWC713::MESSAGE_ID::GENERIC,
                                   "mwc713 responded with empty MWC address" );
            return true;
        }

        wallet713->setMwcAddress(address);
        wallet713->setInitStatus( MWC713::INIT_STATUS::READY );
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::WARNING, MWC713::MESSAGE_ID::GENERIC,"Sucessfully logged in into the wallet");
        return true;
    }

    // static
    QString TaskUnlock::buildWalletRequest(QString password) {
        QString res = "unlock";
        if (password.length() > 0)
            res += " -p " + password;
        return res;
    }

}

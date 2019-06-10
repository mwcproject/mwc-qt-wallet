#include "TaskInitConfirm.h"

#include <QDebug>
#include "../mwc713.h"

namespace wallet {

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

}


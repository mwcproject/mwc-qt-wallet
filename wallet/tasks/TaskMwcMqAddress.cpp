#include "TaskMwcMqAddress.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

bool TaskMwcMqAddress::processTask(const QVector<WEvent> & events) {

    qDebug() << "TaskMwcMqAddress::processTask: " << printEvents(events);

    QVector< WEvent > mwcMqAddr = filterEvents(events, WALLET_EVENTS::S_YOUR_MWC_ADDRESS );
    QVector< WEvent > mwcMqAddrIdx = filterEvents(events, WALLET_EVENTS::S_MWC_ADDRESS_INDEX );

    if (mwcMqAddr.empty()) {
        wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::CRITICAL, MWC713::MESSAGE_ID::GENERIC,
                                             "Unable to get the mwc mq address");
        return true;
    }

    QString address = mwcMqAddr[0].message;

    int idx=-1;

    if (mwcMqAddrIdx.size()>0) {
        bool ok = false;
        idx = mwcMqAddrIdx[0].message.toInt(&ok);
        if (!ok)
            idx=-1;
    }

    qDebug() << "Get mwc MQ address " << address << " for index " << idx;

    if (idx>=0)
        wallet713->setMwcAddressWithIndex( address, idx );

    return true;
}

QString TaskMwcMqAddress::calcCommandLine( bool genNext, int idx ) const {
    if (genNext) {
        if (idx>=0)
            return "config --generate-next-address -i " + QString::number(idx);
        else
            return "config --generate-next-address";
    }
    else
        return "address";
}


}



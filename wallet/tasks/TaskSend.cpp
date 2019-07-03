#include "TaskSend.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../util/address.h"
#include "../../util/stringutils.h"

namespace wallet {

// ---------------- TaskSlatesListener -----------------------

bool TaskSlatesListener::processTask(const QVector<WEvent> & events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];

    switch (evt.event) {
    case S_SLATE_WAS_SENT:{
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()==3) {
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::INFO, MWC713::MESSAGE_ID::GENERIC,
                       "Send slate " + prms[0] + " for " + util::zeroDbl2Dbl( prms[1] ) + " mwc was sent to " + prms[2] );

            wallet713->reportSlateSend( prms[0], util::zeroDbl2Dbl( prms[1] ), prms[2] );
        }
        return true;
    }
    case S_SLATE_WAS_RECEIVED: {
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()==3) {
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::INFO, MWC713::MESSAGE_ID::GENERIC,
                       "Received slate " + prms[0] + " for " + util::zeroDbl2Dbl( prms[2] ) + " mwc from " + prms[1] );
            wallet713->reportSlateRecieved( prms[0], util::zeroDbl2Dbl( prms[2] ), prms[1] );
        }
        return true;
    }
    case S_SLATE_WAS_FINALIZED: {
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()==1) {
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::INFO, MWC713::MESSAGE_ID::GENERIC,
                       "Finalized slate " + prms[0] );
            wallet713->reportSlateFinalized( prms[0] );
        }
        return true;
    }
    default:
        return false;
    }

}

// ---------------- TaskSetReceiveAccount -----------------------

bool TaskSetReceiveAccount::processTask(const QVector<WEvent> &events) {

    QVector< WEvent > okresp = filterEvents(events, WALLET_EVENTS::S_SET_RECEIVE );

    if ( okresp.size()>0 ) {
        wallet713->setSetReceiveAccount( true, okresp[0].message );
        return true;
    }

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    if (errs.size()>0) {
        wallet713->setSetReceiveAccount( false, okresp[0].message );
    }
    else {
        wallet713->setSetReceiveAccount( false, "Unknown error, didn't get expected respond from mwc713" );
    }

    return true;
}

// ------------------------ TaskSendMwc --------------------------------

bool TaskSendMwc::processTask(const QVector<WEvent> &events) {

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );

    if (errs.isEmpty() ) {
        wallet713->setSendResults(true, QStringList());
        return true;
    }

    QStringList errMsgs;

    for (WEvent & evt : errs)
        errMsgs.push_back( evt.message );

    wallet713->setSendResults(false, errMsgs);
    return true;
}


QString TaskSendMwc::buildCommand( MWC713 *wallet713, int64_t coinNano, const QString & address,
        QString message, int inputConfirmationNumber, int changeOutputs ) const {

    QString cmd = "send ";// + util::nano2one(coinNano);
    if (coinNano>0)
        cmd += util::nano2one(coinNano);

        // TODO Message symbols MUST be escaped
    if ( !message.isEmpty() )
        cmd += " --message \"" + message + "\""; // Message symbols MUST be escaped.

    if (inputConfirmationNumber > 0)
        cmd += " --confirmations " + QString::number(inputConfirmationNumber);

    if (changeOutputs > 0)
        cmd += " --change-outputs " + QString::number(changeOutputs);

    // So far documentation doesn't specify difference between protocols
    cmd += " --to " + address;

    if (coinNano<0)
        cmd += " ALL";

    qDebug() << "sendCommand: '" << cmd << "'";

    return cmd;

/*    QPair<bool, util::ADDRESS_TYPE> addressType = util::verifyAddress(address);
    if (!addressType.first) {
        wallet713->setSendResults(false, QStringList{"Unable to parse the address: " + address});
        return "";
    }



    switch (addressType.second) {
        case util::ADDRESS_TYPE::UNKNOWN:
            Q_ASSERT(false);
        case util::ADDRESS_TYPE::MWC_MQ:
        case util::ADDRESS_TYPE::KEYBASE:
        case util::ADDRESS_TYPE::HTTPS:
        default:
            Q_ASSERT(false);
    }

    if ()*/

}

// ----------------------- TaskSendFile --------------------------

bool TaskSendFile::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > res = filterEvents(events, WALLET_EVENTS::S_FILE_TRANS_CREATED );
    if (res.size()>0) {
        QString fn = res[0].message;
        wallet713->setSendFileResult(true, QStringList(), fn);
        return true;
    }

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    QStringList errMsg;
    for (auto & er:errs)
        errMsg.push_back(er.message);

    wallet713->setSendFileResult(false, errMsg, "");
    return true;
}

// ------------------- TaskReceiveFile -----------------------------

bool TaskReceiveFile::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > get = filterEvents(events, WALLET_EVENTS::S_FILE_RECEIVED );
    QVector< WEvent > create = filterEvents(events, WALLET_EVENTS::S_FILE_TRANS_CREATED );

    if (get.size()>0 && create.size()>0) {
        QString getFn = get[0].message;
        QString crFn = create[0].message;

        wallet713->setReceiveFile( true, QStringList(), getFn, crFn );
        return true;
    }

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    QStringList errMsg;
    for (auto & er:errs)
        errMsg.push_back(er.message);

    wallet713->setReceiveFile(false, errMsg, "","");
    return true;
}

// ------------------- TaskFinalizeFile -------------------------

bool TaskFinalizeFile::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > finFn = filterEvents(events, WALLET_EVENTS::S_FILE_TRANS_FINALIZED );

    if (finFn.size()>0 ) {
        QString fn = finFn[0].message;

        wallet713->setFinalizeFile(true, QStringList(), fn );
        return true;
    }

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    QStringList errMsg;
    for (auto & er:errs)
        errMsg.push_back(er.message);

    wallet713->setFinalizeFile(false, errMsg, "");
    return true;

}



}

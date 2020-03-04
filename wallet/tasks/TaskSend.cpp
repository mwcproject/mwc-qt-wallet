// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
    case S_SLATE_WAS_SENT_TO:{
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()==3) {
            wallet713->reportSlateSendTo( prms[0], util::zeroDbl2Dbl( prms[1] ), prms[2] );
        }
        return true;
    }
    case S_SLATE_WAS_SENT_BACK:{
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()==2) {
            wallet713->reportSlateSendBack( prms[0], prms[1] );
        }
        return true;
    }

    case S_SLATE_WAS_RECEIVED_FROM: {
        // We get some moner from somebody!!!
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()>=3) {
            wallet713->reportSlateReceivedFrom( prms[0], util::zeroDbl2Dbl( prms[2] ), prms[1], prms.size()>3 ? prms[3] : "" );
        }
        return true;
    }

    case S_SLATE_WAS_RECEIVED_BACK: {
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()==3) {
            wallet713->reportSlateReceivedBack( prms[0], util::zeroDbl2Dbl( prms[2] ), prms[1] );
        }
        return true;
    }
    case S_SLATE_WAS_FINALIZED: {
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()==1) {
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

    // txid=60
    // slate [2fb9c2a7-abac-4370-b6c8-8e10969902ee] for [0.100000000] MWCs sent successfully to [xmgEvZ4MCCGMJnRnNXKHBbHmSGWQchNr9uZpY5J1XXnsCFS45fsU]

    int64_t txId = -1;
    QString address;
    QString slate;

    {
        QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );
        // Parsing for txId  - index of transaction that was created
        for (auto &ln : lns) {
            if (ln.message.startsWith("txid=")) {
                bool ok = false;
                txId = ln.message.mid(strlen("txid=")).toLongLong(&ok);
                if (!ok)
                    txId = -1;
                else
                    break;
            }
        }
    }

    {
        // Let's check for slate
        QVector< WEvent > sendSlate = filterEvents(events, WALLET_EVENTS::S_SLATE_WAS_SENT_TO);

        if (sendSlate.size()==1) {
            QStringList prms = sendSlate[0].message.split('|');
            if (prms.size()==3) {
                slate = prms[0];
                address = prms[2];
            }
        }
    }

    if ( txId>0 && !slate.isEmpty() && !address.isEmpty() ) {
        wallet713->setSendResults(true, QStringList(), address, txId, slate);
        return true;
    }

    QStringList errMsgs;
    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );

    for (WEvent & evt : errs)
        errMsgs.push_back( evt.message );

    if (errMsgs.isEmpty())
        errMsgs.push_back("Not found expected output from mwc713");

    wallet713->setSendResults( false, errMsgs, "", -1, "" );
    return true;
}

QString TaskSendMwc::buildCommand( int64_t coinNano, const QString & address, const QString & apiSecret,
        QString message, int inputConfirmationNumber, int changeOutputs ) const {

    QString cmd = "send ";// + util::nano2one(coinNano);
    if (coinNano>0)
        cmd += util::nano2one(coinNano);

        // TODO Message symbols MUST be escaped
    if ( !message.isEmpty() )
        cmd += " --message " + util::toMwc713input(message); // Message symbols MUST be escaped.

    if (inputConfirmationNumber > 0)
        cmd += " --confirmations " + QString::number(inputConfirmationNumber);

    if (changeOutputs > 0)
        cmd += " --change-outputs " + QString::number(changeOutputs);

    // So far documentation doesn't specify difference between protocols
    cmd += " --to " + util::toMwc713input(address);

    if (!apiSecret.isEmpty()) {
        cmd += " --apisecret " + util::toMwc713input(apiSecret);
    }

    if (coinNano<0)
        cmd += " ALL";

    qDebug() << "sendCommand: '" << cmd << "'";

    return cmd;
}

// ----------------------- TaskSendFile --------------------------

QString TaskSendFile::buildCommand( int64_t coinNano, QString message, QString fileTx, int inputConfirmationNumber, int changeOutputs ) const {
    QString cmd = "send ";// + util::nano2one(coinNano);
    if (coinNano > 0)
        cmd += util::nano2one(coinNano);

    if (!message.isEmpty())
        cmd += " --message " + util::toMwc713input(message); // Message symbols MUST be escaped.

    if (inputConfirmationNumber > 0)
        cmd += " --confirmations " + QString::number(inputConfirmationNumber);

    if (changeOutputs > 0)
        cmd += " --change-outputs " + QString::number(changeOutputs);

    // So far documentation doesn't specify difference between protocols
    cmd += " --file " + util::toMwc713input(fileTx);

    if (coinNano < 0)
        cmd += " ALL";

    qDebug() << "sendCommand: '" << cmd << "'";

    return cmd;
}

bool TaskSendFile::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    for ( auto & ln : lns ) {
        int idx = ln.message.indexOf("created successfully.");
        if (idx>0) {
            QString fn = ln.message.left(idx).trimmed();
            wallet713->setSendFileResult(true, QStringList(), fn);
            return true;
        }
    }

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    QStringList errMsg;
    for (auto & er:errs)
        errMsg.push_back(er.message);

    wallet713->setSendFileResult(false, errMsg, "");
    return true;
}

// ------------------- TaskReceiveFile -----------------------------

QString TaskReceiveFile::buildCommand(QString fileName, QString identifier) const {
    QString res("receive ");
    res += "--file " + util::toMwc713input(fileName);
    if ( ! identifier.isEmpty() ) {
        res += " -k " + identifier;
    }
    return res;
}


bool TaskReceiveFile::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    int idx=0;
    QString getFn;
    for ( ;idx<lns.size(); idx++ ) {
        int i = lns[idx].message.indexOf("received. amount =");
        if ( i >=0 ) {
            getFn = lns[idx].message.left(i).trimmed();
            break;
        }
    }

    QString crFn;
    for ( ;idx<lns.size(); idx++ ) {
        int i = lns[idx].message.indexOf("created successfully");
        if ( i >=0 ) {
            crFn = lns[idx].message.left(i).trimmed();
            break;
        }
    }

    if ( !getFn.isEmpty() && !crFn.isEmpty() ) {
        wallet713->setReceiveFile( true, QStringList(), getFn, crFn );
        return true;
    }

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    QStringList errMsg;
    for (auto & er:errs)
        errMsg.push_back(er.message);

    wallet713->setReceiveFile(false, errMsg, inFileName,"");
    return true;
}

// ------------------- TaskFinalizeFile -------------------------

bool TaskFinalizeFile::processTask(const QVector<WEvent> &events) {

    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    for ( auto ln : lns ) {
        int idx = ln.message.indexOf(" finalized.");
        if (idx>0) {
            QString fileName = ln.message.left(idx).trimmed();
            wallet713->setFinalizeFile(true, QStringList(), fileName );
            return true;
        }
    }

    QVector< WEvent > apiErrs = filterEvents(events, WALLET_EVENTS::S_NODE_API_ERROR);
    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    QStringList errMsg;
    // We prefer API messages from the node. Wallet doesn't provide details
    for (auto & er:apiErrs) {
        QStringList prms = er.message.split('|');
        if (prms.size()==2)
            errMsg.push_back("MWC-NODE failed to publish the slate. " + prms[1]);
    }
    if (errMsg.isEmpty()) {
        for (auto &er:errs)
            errMsg.push_back(er.message);
    }

    wallet713->setFinalizeFile(false, errMsg, "");
    return true;

}



}

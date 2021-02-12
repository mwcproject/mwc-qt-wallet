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
#include "utils.h"
#include "../../core/Notification.h"

namespace wallet {

// ---------------- TaskSlatesListener -----------------------

bool TaskSlatesListener::processTask(const QVector<WEvent> & events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];

    switch (evt.event) {
    case S_SLATE_WAS_RECEIVED_FROM: {
        // We get some moner from somebody!!!
        qDebug() << "TaskSlatesListener::processTask with events: " << printEvents(events);
        QStringList prms = evt.message.split('|');
        if (prms.size()>=3) {
            wallet713->reportSlateReceivedFrom( prms[0], util::zeroDbl2Dbl( prms[2] ), prms[1], prms.size()>3 ? prms[3] : "" );
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

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR );
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

    // slate [59c6f53f-1230-43a2-bed6-2c7c6100f310] received back from [xmgEvZ4MCCGMJnRnNXKHBbHmSGWQchNr9uZpY5J1XXnsCFS45fsU] for [0.321000000] MWCs
    // txid=34
    // slate [59c6f53f-1230-43a2-bed6-2c7c6100f310] for [0.321000000] MWCs sent successfully to [xmgEvZ4MCCGMJnRnNXKHBbHmSGWQchNr9uZpY5J1XXnsCFS45fsU]

    // Need to know txid and cuccess phrase that 'MWCs sent successfully to'
    int64_t txId = -1;
    QString address;
    QString slate;
    QString mwc;

    {
        QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );
        // Parsing for txId  - index of transaction that was created
        for (auto &ln : lns) {
            if (ln.message.startsWith("txid=")) {
                bool ok = false;
                txId = ln.message.mid(strlen("txid=")).toLongLong(&ok);
                if (!ok)
                    txId = -1;
            }

            if (ln.message.contains("sent successfully to") ) {
                int idx1 = ln.message.indexOf('[');
                int idx2 = ln.message.indexOf(']', idx1+1);
                if (idx1>0 && idx2>0)
                    slate = ln.message.mid(idx1+1, idx2-idx1-1);

                idx1 = ln.message.indexOf("for [");
                idx2 = ln.message.indexOf(']', idx1+1);
                if (idx1>0 && idx2>0) {
                    idx1 += int(strlen("for ["));
                    mwc = ln.message.mid(idx1, idx2 - idx1);
                    mwc = util::zeroDbl2Dbl(mwc);
                }

                idx1 = ln.message.lastIndexOf('[');
                idx2 = ln.message.indexOf(']', idx1+1);
                if (idx1>0 && idx2>0)
                    address = ln.message.mid(idx1+1, idx2-idx1-1);
            }
        }
    }

    if ( txId>0 && !slate.isEmpty() && !address.isEmpty() && !mwc.isEmpty() ) {
        wallet713->setSendResults(true, QStringList(), address, txId, slate, mwc);
        return true;
    }

    QStringList errMsgs;
    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR );

    for (WEvent & evt : errs) {
        if (evt.message.contains("is recipient listening"))
            errMsgs.push_back("Recipient wallet is offline. Please retry to send when recipient wallet will be online.");

        errMsgs.push_back(evt.message);
    }

    if (errMsgs.isEmpty())
        errMsgs.push_back("Not found expected output from mwc713");

    wallet713->setSendResults( false, errMsgs, "", -1, "", "" );
    return true;
}

QString TaskSendMwc::buildCommand( int64_t coinNano, const QString & address, const QString & apiSecret,
        QString message, int inputConfirmationNumber, int changeOutputs,
        const QStringList & outputs, bool fluff, int ttl_blocks,
        bool generateProof, const QString & expectedproofAddress ) const {

    QString cmd = "send ";// + util::nano2one(coinNano);
    if (coinNano>0)
        cmd += util::nano2one(coinNano);

        // TODO Message symbols MUST be escaped
    if ( !message.isEmpty() )
        cmd += " --message " + util::toMwc713input(message); // Message symbols MUST be escaped.

    if (!outputs.isEmpty()) {
        cmd += " --confirmations 1 --strategy custom --outputs " + outputs.join(",");
    }

    if (outputs.isEmpty() && inputConfirmationNumber > 0)
        cmd += " --confirmations " + QString::number(inputConfirmationNumber);

    if (changeOutputs > 0)
        cmd += " --change-outputs " + QString::number(changeOutputs);

    // So far documentation doesn't specify difference between protocols
    cmd += " --to " + util::toMwc713input(address);

    if (!apiSecret.isEmpty()) {
        cmd += " --apisecret " + util::toMwc713input(apiSecret);
    }

    if (fluff) {
        cmd += " --fluff";
    }

    if (ttl_blocks > 0)
        cmd += " --ttl-blocks " + QString::number(ttl_blocks);

    if (generateProof)
        cmd += " --proof";

    if (!expectedproofAddress.isEmpty()) {
        cmd += " --expectedproof " + expectedproofAddress;
    }

    if (coinNano<0)
        cmd += " ALL";

    qDebug() << "sendCommand: '" << cmd << "'";

    return cmd;
}

// ----------------------- TaskSendFile --------------------------

QString TaskSendFile::buildCommand( int64_t coinNano, QString message, QString fileTx, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, int ttl_blocks, bool generateProof) const {
    QString cmd = "send ";// + util::nano2one(coinNano);
    if (coinNano > 0)
        cmd += util::nano2one(coinNano);

    if (!message.isEmpty())
        cmd += " --message " + util::toMwc713input(message); // Message symbols MUST be escaped.

    if (!outputs.isEmpty()) {
        cmd += " --confirmations 1 --strategy custom --outputs " + outputs.join(",");
    }

    if (outputs.isEmpty() && inputConfirmationNumber > 0)
        cmd += " --confirmations " + QString::number(inputConfirmationNumber);

    if (changeOutputs > 0)
        cmd += " --change-outputs " + QString::number(changeOutputs);

    // So far documentation doesn't specify difference between protocols
    cmd += " --file " + util::toMwc713input(fileTx);

    if (ttl_blocks > 0)
        cmd += " --ttl-blocks " + QString::number(ttl_blocks);

    if (generateProof)
        cmd += " --proof";

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

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR );
    QStringList errMsg;
    for (auto & er:errs)
        errMsg.push_back(er.message);

    wallet713->setSendFileResult(false, errMsg, "");
    return true;
}

// ------------------- TaskReceiveFile -----------------------------

QString TaskReceiveFile::buildCommand(QString fileName, QString description, QString identifier) const {
    QString res("receive ");
    res += "--file " + util::toMwc713input(fileName);
    if (!description.isEmpty()) {
        res += " --message " + util::toMwc713input(description);
    }
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

    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR );
    QStringList errMsg;
    for (auto & er:errs)
        errMsg.push_back(er.message);

    // Update possibe error meesage
    for ( QString & s : errMsg ) {
        if (s.contains("Transaction Expired", Qt::CaseInsensitive)) {
            s = "This transaction has expired TTL. It already cancelled by sender wallet.";
        }
    }

    wallet713->setReceiveFile(false, errMsg, inFileName,"");
    return true;
}

// ------------------- TaskFinalizeFile -------------------------

QString TaskFinalizeFile::buildCommand(QString fileName, bool fluff) const {
    QString res("finalize ");
    res += "--file " + util::toMwc713input(fileName);
    if (fluff) {
        res += " --fluff";
    }
    return res;
}

bool TaskFinalizeFile::processTask(const QVector<WEvent> &events) {

    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    //  txp1.tx.response finalized transaction edaf5197-677e-48eb-a414-db377896a8c3
    for ( auto ln : lns ) {
        int idx = ln.message.indexOf(" finalized ");
        if (idx>0) {
            QString fileName = ln.message.left(idx).trimmed();
            wallet713->setFinalizeFile(true, QStringList(), fileName );
            return true;
        }
    }

    QVector< WEvent > apiErrs = filterEvents(events, WALLET_EVENTS::S_NODE_API_ERROR);
    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR );
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

// ----------------------- TaskSendSlatepack ---------------------------------

QString TaskSendSlatepack::buildCommand( int64_t coinNano, QString message, int inputConfirmationNumber, int changeOutputs,
                                         const QStringList & outputs, int ttl_blocks, bool generateProof, QString slatepackRecipientAddress,
                                         bool isLockLater ) const {
    QString cmd = "send ";
    if (coinNano > 0)
        cmd += util::nano2one(coinNano);

    if (!message.isEmpty())
    cmd += " --message " + util::toMwc713input(message); // Message symbols MUST be escaped.

    if (!outputs.isEmpty()) {
        cmd += " --confirmations 1 --strategy custom --outputs " + outputs.join(",");
    }

    if (outputs.isEmpty() && inputConfirmationNumber > 0)
        cmd += " --confirmations " + QString::number(inputConfirmationNumber);

    if (changeOutputs > 0)
        cmd += " --change-outputs " + QString::number(changeOutputs);

    // So far documentation doesn't specify difference between protocols
    if (slatepackRecipientAddress.isEmpty())
        cmd += " --slatepack";
    else
        cmd += " --slatepack_recipient " + slatepackRecipientAddress;

    if (ttl_blocks > 0)
        cmd += " --ttl-blocks " + QString::number(ttl_blocks);

    if (generateProof)
        cmd += " --proof";

    if (isLockLater)
        cmd += " --lock_later";

    if (coinNano < 0)
        cmd += " ALL";

    qDebug() << "send slatepack Command: '" << cmd << "'";
    return cmd;
}

bool TaskSendSlatepack::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    for ( auto & ln : lns ) {
        if (ln.message.startsWith("Slatepack: ")) {
            QString slatepack = ln.message.mid( strlen("Slatepack: ") ).trimmed();
            wallet713->setSendSlatepack("", slatepack, tag);
            return true;
        }
    }

    wallet713->setSendSlatepack(getErrorMessage(events, "Unable to get a slate"), "", tag);
    return true;
}

// ---------------------------- TaskReceiveSlatepack -----------------------------

QString TaskReceiveSlatepack::buildCommand(QString slatepack, QString description) const {
    QString cmd = "receive --content " + util::toMwc713input(slatepack);
    if (!description.isEmpty()) {
        cmd += " -m " + util::toMwc713input(description);
    }
    return cmd;
}


bool TaskReceiveSlatepack::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    for ( auto & ln : lns ) {
        if (ln.message.startsWith("Slatepack: ")) {
            QString slatepack = ln.message.mid( strlen("Slatepack: ") ).trimmed();
            wallet713->setReceiveSlatepack("", slatepack, tag);
            return true;
        }
    }

    wallet713->setReceiveSlatepack(getErrorMessage(events, "Unable to get a slate"), "", tag);
    return true;
}

// --------------------------- TaskFinalizeSlatepack ------------------------------------

QString TaskFinalizeSlatepack::buildCommand(QString slatepack, bool fluff) const {
    QString res("finalize ");
    res += "--content \"" + slatepack + "\"";
    if (fluff) {
        res += " --fluff";
    }
    return res;
}

bool TaskFinalizeSlatepack::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    for ( auto & ln : lns ) {
        int idx = ln.message.indexOf(" finalized transaction ");
        if (idx>=0) {
            idx += strlen(" finalized transaction ");
            QString txId = ln.message.mid(idx).trimmed();
            wallet713->setFinalizedSlatepack("", txId, tag);
            return true;
        }
    }

    wallet713->setFinalizedSlatepack(getErrorMessage(events, "Unable to finalize slatepack"), "", tag);
    return true;
}

// --------------------------- TaskRequestRecieverWalletAddress ---------------------------

bool TaskRequestRecieverWalletAddress::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    for (const auto & ln : lns) {
        const QString msg = ln.message;
        if (msg.startsWith("Proof pub key of the listening wallet:")) {
            QString addr = msg.mid( strlen("Proof pub key of the listening wallet:") ).trimmed();
            wallet713->setRequestRecieverWalletAddress(url, addr, "");
            return true;
        }
    }

    // reporting error
    wallet713->setRequestRecieverWalletAddress(url, "", getErrorMessage(events, "Unable to get a receiver wallet address for " + url));
    return true;
}

// ---------------------------------- TaskDecodeSlatepack -------------------------------

void TaskDecodeSlatepack::onStarted() {
    notify::addFalseMessage("Unable to decode the slatepack");
}


bool TaskDecodeSlatepack::processTask(const QVector<WEvent> &events) {
    // Normally we have 3 lines.
    // Slate: {"version_info":{"version":3, ...
    // Content: InvoiceInitial
    // Sender: None

    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    QString slate;
    QString content;
    QString sender;
    QString recipient;

    for (const auto & ln : lns) {
        const QString msg = ln.message;
        if (msg.startsWith("Slate: ")) {
            slate = msg.mid(strlen("Slate: ")).trimmed();
        }
        else if ( msg.startsWith("Content: ") ) {
            content = msg.mid(strlen("Content: ")).trimmed();
        }
        else if ( msg.startsWith("Sender: ") ) {
            sender = msg.mid(strlen("Sender: ")).trimmed();
        }
        else if ( msg.startsWith("Recipient: ") ) {
            recipient = msg.mid(strlen("Recipient: ")).trimmed();
        }

    }

    notify::remeoveFalseMessage("Unable to decode the slatepack");

    if (slate.isEmpty() || content.isEmpty() || sender.isEmpty()) {
        wallet713->setDecodeSlatepack( tag, getErrorMessage(events, "Unable to decode a slatepack"), slatepack, "","","", "");
    }
    else {
        wallet713->setDecodeSlatepack( tag, "", slatepack, slate, content, sender, recipient);
    }
    return true;
}


}

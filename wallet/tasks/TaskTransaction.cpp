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

#include "TaskTransaction.h"
#include "../mwc713.h"
#include <QDebug>
#include "../../core/HodlStatus.h"

namespace wallet {

// ------------------------------------ TaskOutputs -------------------------------------------

struct OutputIdxLayout {
    int outputCommitment = -1;
    int mmrIndex = -1;
    int blockHeight = -1;
    int lockedUntil = -1;
    int status = -1;
    int coinbase = -1;
    int confirms= -1;
    int value = -1;
    int tx = -1;

    bool isDefined() const {
        return outputCommitment>=0 && mmrIndex>0 && blockHeight>0 && lockedUntil>0 && status >0 && coinbase>0 && confirms>0 && value>0 && tx>0;
    }
};

static bool parseOutputLine( QString str, const OutputIdxLayout & tl,
                           WalletOutput & output) {

    QString strOutputCommitment = util::getSubString(str, tl.outputCommitment, tl.mmrIndex );
    QString strMmrIndex = util::getSubString(str, tl.mmrIndex, tl.blockHeight);
    QString strBlockHeight = util::getSubString(str, tl.blockHeight, tl.lockedUntil);
    QString strLockedUntil = util::getSubString(str, tl.lockedUntil, tl.status);
    QString strStatus = util::getSubString(str, tl.status, tl.coinbase);
    QString strCoinbase = util::getSubString(str, tl.coinbase, tl.confirms);
    QString strConfirms = util::getSubString(str, tl.confirms, tl.value);
    QString strValue = util::getSubString(str, tl.value, tl.tx);
    QString strTx = util::getSubString(str, tl.tx, str.length());

    if (strOutputCommitment.isEmpty() )
        return false;

    QPair<bool,int64_t> mwcOne = util::one2nano(strValue);

    bool ok = false;
    int64_t tx = strTx.toLongLong(&ok);
    if (!ok)
        return false;

    output.setData(strOutputCommitment,
                   strMmrIndex,
                   strBlockHeight,
            strLockedUntil,
            strStatus,
            strCoinbase != "false",
            strConfirms,
            mwcOne.second,
            tx);

    return mwcOne.first;
}

static int getNumberFromEvents(const QVector<WEvent> & events) {
    // Searching for the last line with just a number
    int number = 0; // in case if not found...
    for (int i=events.size()-1; i>=0; i--) {
        if ( events[i].event == WALLET_EVENTS::S_LINE ) {
            bool ok = false;
            int n = events[i].message.toInt(&ok);
            if (ok && n>=0) {
                number = n;
                break;
            }
        }
    }
    return number;
}


bool TaskOutputCount::processTask(const QVector<WEvent> & events) {
    wallet713->updateOutputCount( account, getNumberFromEvents(events));
    return true;
}

bool TaskTransactionCount::processTask(const QVector<WEvent> & events) {
    wallet713->updateTransactionCount(account, getNumberFromEvents(events));
    return true;
}

static void parseOutputData( const QVector<WEvent> & events, // in
        QString & account, int64_t & height, QVector<WalletOutput> & outputs ) // out
{
    int curEvt = 0;

    account = "";
    height = -1;
    outputs.clear();

    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_OUTPUT_LOG ) {
            QStringList l = events[curEvt].message.split('|');
            Q_ASSERT(l.size()==2);
            account = l[0];
            height = l[1].toInt();
            break;
        }
    }

    // positions for the columns. Note, the columns and the order are hardcoded and come from mwc713 data!!!

    OutputIdxLayout outLayout;

    // Continue with transaction output
    // Search for Header first
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE ) {
            auto & str = events[curEvt].message;
            if ( str.contains("Output Commitment") && str.contains("Block Height") ) {

                outLayout.outputCommitment = str.indexOf("Output Commitment");
                outLayout.mmrIndex = str.indexOf("MMR Index", outLayout.outputCommitment);
                outLayout.blockHeight = str.indexOf("Block Height", outLayout.mmrIndex);
                outLayout.lockedUntil = str.indexOf("Locked Until", outLayout.blockHeight);
                outLayout.status = str.indexOf("Status", outLayout.lockedUntil);
                outLayout.coinbase = str.indexOf("Coinbase", outLayout.status);
                outLayout.confirms= str.indexOf("# Confirms", outLayout.coinbase);
                outLayout.value = str.indexOf("Value", outLayout.confirms);
                outLayout.tx = str.indexOf("Tx", outLayout.value);

                if ( outLayout.isDefined() ) {
                    curEvt++;
                    break;
                }
                Q_ASSERT(false); // There is a small chance, but it is really not likely it is ok
            }
        }
    }

    // Skip header
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE) {
            auto &str = events[curEvt].message;
            if (str.startsWith("==============")) {
                curEvt++; // skipping this
                break;
            }
        }
    }

    // Processing transactions
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE ) {
            auto &str = events[curEvt].message;

            if (str.startsWith("--------------------"))
                continue;

            // Expected to be a normal line
            WalletOutput outpt;
            if ( parseOutputLine(str, outLayout, outpt) ) {
                outputs.push_back(outpt);
            }
        }
    }
}


bool TaskOutputs::processTask(const QVector<WEvent> & events) {
    // We are processing transactions outptu mostly as a raw data

    QString account;
    int64_t  height = -1;
    QVector< WalletOutput > outputResult;

    parseOutputData( events, // in
        account, height, outputResult );

    wallet713->setOutputs(account, height, outputResult );
    return true;
}

bool TaskOutputsForHODL::processTask(const QVector<WEvent> & events) {
    Q_ASSERT(hodlStatus);

    QString account;
    int64_t  height = -1;
    QVector< WalletOutput > outputResult;

    parseOutputData( events, // in
                     account, height, outputResult );

    hodlStatus->setWalletOutputs( account, outputResult, "walletOutputs" );
    return true;
}


// ------------------------------------ TaskTransactions -------------------------------------------

struct TransactionIdxLayout {
    int posId = -1;
    int posType = -1;
    int posTxid = -1;
    int posAddress = -1;
    int posCrTime = -1;
    int posConf = -1;
    int posHeight = -1;
    int posConfTime = -1;
    int posNetDiff = -1;
    int posProof = -1;

    bool isDefined() const {
        return posId>=0 && posType>0 && posTxid>0 && posAddress>0 && posCrTime>0 &&
                            posConf>0 && posHeight>0 && posConfTime>0 && posNetDiff>0 && posProof>0;
    }
};

bool parseTransactionLine( QString str, const TransactionIdxLayout & tl,
                           WalletTransaction & trans) {

    QString strId       = util::getSubString(str, tl.posId, tl.posType);
    QString strType     = util::getSubString(str, tl.posType, tl.posTxid);
    QString strTxid     = util::getSubString(str, tl.posTxid, tl.posAddress);
    QString strAddress  = util::getSubString(str, tl.posAddress, tl.posCrTime);
    QString strCrTime   = util::getSubString(str, tl.posCrTime, tl.posConf);
    QString strConf     = util::getSubString(str, tl.posConf, tl.posHeight);
    QString strHeight   = util::getSubString(str, tl.posHeight, tl.posConfTime);
    QString strConfTime = util::getSubString(str, tl.posConfTime, tl.posNetDiff);
    QString strNetDiff  = util::getSubString(str, tl.posNetDiff, tl.posProof);
    QString strProof    = util::getSubString(str, tl.posProof, str.length());

    if (strId.length() == 0)
        return false;

    bool ok = false;
    int64_t id = -1;
    id = strId.toLongLong(&ok);
    if (!ok || id < 0)
        return false;

    WalletTransaction::TRANSACTION_TYPE tansType = WalletTransaction::TRANSACTION_TYPE::NONE;
    if (strType.startsWith("Sent") || strType.startsWith("Send"))
        tansType = WalletTransaction::TRANSACTION_TYPE::SEND;
    else if (strType.startsWith("Received"))
        tansType = WalletTransaction::TRANSACTION_TYPE::RECEIVE;
    else
        return false;

    if ( strCrTime.isEmpty())
        return false;

    // At v3.0.0 we have: true, false
    bool conf = strConf.startsWith("yes", Qt::CaseInsensitive) || strConf.startsWith("true", Qt::CaseInsensitive);

    int64_t height = strHeight.isEmpty() ? 0 : strHeight.toLongLong();

    QPair<bool, int64_t> net = util::one2nano(strNetDiff);
    if ( !net.first )
        return false;

    // At v3.0.0 we have: Yes,None
    bool proof = strProof.startsWith("yes", Qt::CaseInsensitive) || strProof.startsWith("true", Qt::CaseInsensitive);

    trans.setData(id, tansType,
                  strTxid,
                  strAddress,
                  strCrTime,
                  conf,
                  height,
                  strConfTime,
                  net.second,
                  proof);
    return true;
}

// local utility function that parse transactions output
static void parseTransactionsOutput(const QVector<WEvent> & events, // in
                                    QString & account, // out
                                    int64_t & height,  // out
                                    QVector<WalletTransaction> & trVector) // out
{
    // We are processing transactions mostly as a raw data
    int curEvt = 0;

    account = "";
    height = -1;

    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_TRANSACTION_LOG ) {
            QStringList l = events[curEvt].message.split('|');
            Q_ASSERT(l.size()==2);
            account = l[0];
            height = l[1].toInt();
            break;
        }
    }

    // positions for the columns. Note, the columns and the order are hardcoded and come from mwc713 data!!!

    TransactionIdxLayout tl;

    // Continue with transaction output
    // Search for Header first
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE ) {
            auto & str = events[curEvt].message;
            if ( str.contains("Creation Time") && str.contains("Confirmed?") ) {
                tl.posId = str.indexOf("Id");
                tl.posType = str.indexOf("Type", tl.posId);
                tl.posTxid = str.indexOf("TXID", tl.posType);
                tl.posAddress = str.indexOf("Address", tl.posTxid);
                tl.posCrTime = str.indexOf("Creation Time", tl.posAddress);
                tl.posConf = str.indexOf("Confirmed?", tl.posCrTime);
                tl.posHeight = str.indexOf("Height", tl.posConf);
                tl.posConfTime = str.indexOf("Confirmation Time", tl.posConf);
                tl.posNetDiff = str.indexOf("Net", tl.posConfTime);
                tl.posProof = str.indexOf("Proof?", tl.posNetDiff);

                if ( tl.isDefined() ) {
                    curEvt++;
                    break;
                }
                Q_ASSERT(false); // There is a small chance, but it is really not likely it is ok
            }
        }
    }

    // Skip header
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE) {
            auto &str = events[curEvt].message;
            if (str.startsWith("==============")) {
                curEvt++; // skipping this
                break;
            }
        }
    }

    QMap<int64_t, WalletTransaction > transactions;

    // Processing transactions
    int64_t lastTransId = -1;
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE ) {
            auto &str = events[curEvt].message;

            if (str.startsWith("--------------------"))
                continue;

            // mwc713 has a special line for 'cancelled'
            if (str.contains("- Cancelled")) {
                transactions[lastTransId].cancelled();
                continue;
            }

            // Expected to be a normal line
            WalletTransaction trans;
            if ( parseTransactionLine(str, tl, trans) ) {
                lastTransId = trans.txIdx;
                transactions[trans.txIdx] = trans;
            }
        }
    }

    trVector.clear();
    for ( WalletTransaction & trItem : transactions )
        trVector.push_back( trItem );
}

bool TaskTransactions::processTask(const QVector<WEvent> & events) {

    QString account;
    int64_t height = -1;
    QVector<WalletTransaction> trVector;

    parseTransactionsOutput(events, // in
                           account, height, trVector); // out

    wallet713->setTransactions( account, height, trVector );
    return true;
}



// Just a callback, not a real task
bool TaskAllTransactionsStart::processTask(const QVector<WEvent> &events) {
    Q_UNUSED(events)
    wallet713->processAllTransactionsStart();
    return true;
}

bool TaskAllTransactionsEnd::processTask(const QVector<WEvent> &events) {
    Q_UNUSED(events)
    wallet713->processAllTransactionsEnd();
    return true;
}

bool TaskAllTransactions::processTask(const QVector<WEvent> & events) {
    QString account;
    int64_t height = -1;
    QVector<WalletTransaction> trVector;

    parseTransactionsOutput(events, // in
                            account, height, trVector); // out

    wallet713->processAllTransactionsAppend( trVector );

    return true;
}



// ------------------------- TaskTransCancel ---------------------------

bool TaskTransCancel::processTask(const QVector<WEvent> & events) {
    QVector< WEvent > errors = filterEvents(events, WALLET_EVENTS::S_ERROR );
    if (errors.isEmpty()) {
        wallet713->setTransCancelResult( true, transactionId, "" );
    }
    else {
        QStringList messages;
        for (auto & e : errors)
            messages.push_back(e.message);
        wallet713->setTransCancelResult( false, transactionId, util::formatErrorMessages(messages) );
    }
    return true;
}

// ------------------------------------ TaskTransExportProof -------------------------------------------

bool TaskTransExportProof::processTask(const QVector<WEvent> & events) {
    QVector< WEvent > errors = filterEvents(events, WALLET_EVENTS::S_ERROR );
    if (!errors.isEmpty()) {
        QStringList messages;
        for (auto & e : errors)
            messages.push_back(e.message);
        // respond back with error
        wallet713->setExportProofResults(false, "", "Unable to export proof for transactions "+ QString::number(transactionId) +".\n" + util::formatErrorMessages(messages));
        return true;
    }

    QVector< WEvent > lines = filterEvents(events, WALLET_EVENTS::S_LINE );

    QString fileName;

    int evtIdx = 0;
    const QString writtenKeyPhrase("proof written to ");
    for ( ; evtIdx<lines.size(); evtIdx++ ) {
        QString & str = lines[evtIdx].message;
        if (str.contains(writtenKeyPhrase)) {
            fileName = lines[evtIdx].message.mid( str.indexOf(writtenKeyPhrase) + writtenKeyPhrase.length() ).trimmed();
            qDebug() << "Found proof file name: " << fileName;
            evtIdx++;
            break;
        }
    }

    if ( fileName.isEmpty() ) {
        wallet713->setExportProofResults(false, "", "mwc713 return unexpected results.");
        return true;
    }

    QString proofMsg;

    // Process all lines until prompt
    for ( ; evtIdx<lines.size(); evtIdx++ ) {
        QString & str = lines[evtIdx].message;
        if (str.contains( mwc::PROMPTS_MWC713 )) {
            break;
        }

        if (!proofMsg.isEmpty())
            proofMsg += "\n";

        proofMsg += str;
    }

    qDebug() << "Calling setExportProofResults with success";
    wallet713->setExportProofResults(true, fileName, proofMsg);
    return true;
}

// ------------------------------------ TaskTransVerifyProof -------------------------------------------



bool TaskTransVerifyProof::processTask(const QVector<WEvent> & events) {
    QVector< WEvent > errors = filterEvents(events, WALLET_EVENTS::S_ERROR );
    if (!errors.isEmpty()) {
        QStringList messages;
        for (auto & e : errors)
            messages.push_back(e.message);
        // respond back with error
        wallet713->setVerifyProofResults(false, proofFileName, "Unable to verify proof for file "+ proofFileName +".\n" + util::formatErrorMessages(messages));
        return true;
    }

    QVector< WEvent > lines = filterEvents(events, WALLET_EVENTS::S_LINE );

    QString proofMsg;

    // Process all lines until prompt
    for ( auto & ln : lines ) {
        QString & str = ln.message;
        if (str.contains( mwc::PROMPTS_MWC713 )) {
            break;
        }

        if (!proofMsg.isEmpty())
            proofMsg += "\n";

        proofMsg += str;
    }

    qDebug() << "Calling setExportProofResults with success";
    wallet713->setVerifyProofResults(true, proofFileName, proofMsg);
    return true;
}


}

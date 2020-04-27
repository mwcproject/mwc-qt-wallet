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

////////////////////// Common routine ///////////////////////////////////////////////////

static QVector<int> parseHeadersLine( const QString & str, const QVector<QString> & headers ) {
    Q_ASSERT(headers.size()>0);

    QVector<int> res;

    int curPos = 0;
    for ( const auto & hdr : headers ) {
        int pos = str.indexOf(hdr, curPos);
        if (pos<0)
            break;
        res.push_back(pos);
        curPos = pos + hdr.length();
    }

    if (res.size()<headers.size())
        res.clear();

    return res;
}

static QVector<QString> parseDataLine( const QString & str, const QVector<int> & offsets ) {
    Q_ASSERT(offsets.size()>0);

    QVector<QString> res;

    for (int i=0; i<offsets.size(); i++) {
        int idx1 = offsets[i];
        int idx2 = str.length();
        if (i+1<offsets.size())
            idx2 = offsets[i+1];

        res.push_back(util::getSubString(str, idx1, idx2));
    }
    return res;
}




// ------------------------------------ TaskOutputs -------------------------------------------

WalletOutput parseOutputLine( QString str, const QVector<int> & outputLayout) {

    WalletOutput res; // invalid until data is set

    QVector<QString> values = parseDataLine(str, outputLayout);
    if (values.isEmpty())
        return res;

    Q_ASSERT(values.size()==9);

    QString strOutputCommitment = values[0];
    QString strMmrIndex     = values[1];
    QString strBlockHeight  = values[2];
    QString strLockedUntil  = values[3];
    QString strStatus       = values[4];
    QString strCoinbase     = values[5];
    QString strConfirms     = values[6];
    QString strValue        = values[7];
    QString strTx           = values[8];

    QPair<bool,int64_t> mwcOne = util::one2nano(strValue);

    bool ok = false;
    int64_t tx = strTx.toLongLong(&ok);

    if ( strOutputCommitment.isEmpty() || !ok || !mwcOne.first )
        return res;

    res.setData(strOutputCommitment,
                   strMmrIndex,
                   strBlockHeight,
            strLockedUntil,
            strStatus,
            strCoinbase != "false",
            strConfirms,
            mwcOne.second,
            tx);

    return res;
}


static void parseOutputs(const QVector<WEvent> & events, // in
                              QString & account, // out
                              int64_t & height,  // out
                              QVector<WalletOutput> & outputVector) // out
{
    int curEvt = 0;

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

    QVector<int> outputLayout;

    // Continue with transaction output
    // Search for Header first
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE ) {
            auto & str = events[curEvt].message;
            if ( str.contains("Output Commitment") && str.contains("Block Height") ) {

                outputLayout = parseHeadersLine( str, {"Output Commitment", "MMR Index", "Block Height",
                                                       "Locked Until", "Status", "Coinbase?", "# Confirms", "Value", "Tx"} );

                if ( outputLayout.size()>0 ) {
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

            if (str.startsWith("=============="))
                break; // multiple data types case, nned to handle without surprises

            // Expected to be a normal line
            WalletOutput output = parseOutputLine(str, outputLayout);
            if ( output.isValid() ) {
                outputVector.push_back(output);
            }
        }
    }
}

bool TaskOutputs::processTask(const QVector<WEvent> & events) {
    // We are processing transactions outptu mostly as a raw data

    QString account;
    int64_t  height = -1;
    QVector< WalletOutput > outputResult;

    parseOutputs(events, // in
           account, height, outputResult); // out

    wallet713->setOutputs(account, height, outputResult );
    return true;
}

bool TaskOutputsForAccount::processTask(const QVector<WEvent> & events) {

    QString account;
    int64_t  height = -1;
    QVector< WalletOutput > outputResult;

    parseOutputs( events, // in
                     account, height, outputResult );

    wallet713->setWalletOutputs( account, outputResult);
    return true;
}


// ------------------------------------ TaskTransactions -------------------------------------------

static WalletTransaction parseTransactionLine( const QString & str, const QVector<int> & txLayout) {

    Q_ASSERT(txLayout.size()==18);

    WalletTransaction res; // invalid until data is set

    QVector<QString> values = parseDataLine(str, txLayout);
    if (values.isEmpty())
        return res;

    Q_ASSERT(values.size()==18);

    QString strId       = values[0];
    QString strType     = values[1];
    QString strTxid     = values[2];
    QString strAddress  = values[3];
    QString strCrTime   = values[4];
    QString strTtlCutOff = values[5];
    QString strConf     = values[6];
    QString strHeight   = values[7];
    QString strConfTime = values[8];
    QString strNumInputs = values[9];
    QString strNumOutputs = values[10];
    QString strCredited = values[11];
    QString strDebited  = values[12];
    QString strFee      = values[13];
    QString strNetDiff  = values[14];
    QString strProof    = values[15];
    QString strKernel   = values[16];
    // Last 'Tx Data' we don't need

    bool txIdx_ok = false;
    int64_t txIdx = strId.toLongLong(&txIdx_ok);

    WalletTransaction::TRANSACTION_TYPE tansType = WalletTransaction::TRANSACTION_TYPE::NONE;
    if (strType.startsWith("Sent") || strType.startsWith("Send"))
        tansType = WalletTransaction::TRANSACTION_TYPE::SEND;
    else if (strType.startsWith("Received"))
        tansType = WalletTransaction::TRANSACTION_TYPE::RECEIVE;
    else if (strType.startsWith("Confirmed"))
        tansType = WalletTransaction::TRANSACTION_TYPE::COIN_BASE;

    int64_t height = strHeight.isEmpty() ? 0 : strHeight.toLongLong();

    int64_t ttlCutOff = strHeight.isEmpty() || strHeight=="None" ? -1 :  strTtlCutOff.toLongLong();

    int numInputs = strNumInputs.isEmpty() ? -1 : strNumInputs.toInt();
    int numOutputs = strNumOutputs.isEmpty() ? -1 : strNumOutputs.toInt();

    // one2nano return 0 if not able to parce. It is what we need for empty values
    QPair<bool, int64_t> credited = util::one2nano(strCredited);
    QPair<bool, int64_t> debited = util::one2nano(strDebited);
    QPair<bool, int64_t> fee = util::one2nano(strFee);
    QPair<bool, int64_t> net = util::one2nano(strNetDiff);

    // Check if critical fields was prsed correctly. Other fields can be invalid.
    if ( strId.isEmpty() || !txIdx_ok || txIdx < 0 || strCrTime.isEmpty() || !net.first )
        return res;

    res.setData(txIdx,
            tansType,
            strTxid,
            strAddress,
            strCrTime,
            strConf.startsWith("yes", Qt::CaseInsensitive) || strConf.startsWith("true", Qt::CaseInsensitive),
            ttlCutOff,
            height,
            strConfTime,
            numInputs,
            numOutputs,
            credited.second,
            debited.second,
            fee.second,
            net.second,
            strProof.startsWith("yes", Qt::CaseInsensitive) || strProof.startsWith("true", Qt::CaseInsensitive),
            strKernel);

    return res;
}

// local utility function that parse transactions output
static void parseTransactions(const QVector<WEvent> & events, // in
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

    QVector<int> txLayout;

    // Continue with transaction output
    // Search for Header first
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE ) {
            auto & str = events[curEvt].message;
            if ( str.contains("Creation Time") && str.contains("Confirmed?") ) {

                txLayout = parseHeadersLine( str, {"Id", "Type", "Shared Transaction Id", "Address", "Creation Time",
                               "TTL Cutoff Height", "Confirmed?", "Height", "Confirmation Time",  "Num.",  "Num.", "Amount", "Amount", "Fee", "Net", "Payment", "Kernel", "Tx"} );

                if ( txLayout.size()>0 ) {
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

            if (str.startsWith("=============="))
                break; // multiple data types case, nned to handle without surprises


            // mwc713 has a special line for 'cancelled'
            if (str.contains("- Cancelled")) {
                transactions[lastTransId].cancelled();
                continue;
            }

            // Expected to be a normal line
            WalletTransaction trans = parseTransactionLine(str, txLayout);
            if ( trans.isValid() ) {
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

    parseTransactions(events, // in
                           account, height, trVector); // out

    wallet713->setTransactions( account, height, trVector );
    return true;
}

// Just parse the messages
static void parseMessages(const QVector<WEvent> & events, // in
                         QVector<QString> & messages) // out
{
    int curEvt = 0;

    // positions for the columns. Note, the columns and the order are hardcoded and come from mwc713 data!!!

    QVector<int> messagesLayout;

    // Continue with transaction output
    // Search for Header first
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE ) {
            auto & str = events[curEvt].message;
            if ( str.contains("Participant Id") &&  str.contains("Message") && str.contains("Public Key") && str.contains("Signature") ) {

                messagesLayout = parseHeadersLine( str, {"Participant Id", "Message", "Public Key" , "Signature"} );

                if ( messagesLayout.size()>0 ) {
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

    // Processing transactions messages
    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_LINE ) {
            auto &str = events[curEvt].message;

            if (str.startsWith("--------------------"))
                continue;

            if (str.startsWith("=============="))
                break; // multiple data types case, nned to handle without surprises

            QVector<QString> values = parseDataLine(str, messagesLayout);
            if (values.isEmpty())
                continue;

            Q_ASSERT(values.size()==4);

            if ( values[3].length()<140 )
                continue;

            // We don't care about other fields, just messages
            const QString & strMessage = values[1];

            if (strMessage.isEmpty() || strMessage.compare("None", Qt::CaseInsensitive)==0)
                continue;

            messages.push_back(strMessage);
        }
    }
}


bool TaskTransactionsById::processTask(const QVector<WEvent> & events) {
    // Parcing Transaction data. Expected 1 item into the list
    // In not found - there will be empty data, no errors/warnings will come from the wallet

    QString account;
    int64_t height = -1;
    QVector<WalletTransaction> trVector;

    parseTransactions(events, // in
         account, height, trVector); // out

    if ( trVector.size()!=1 ) {
        wallet713->setTransactionById(false,  account, height, WalletTransaction(), {}, {} );
        return true;
    }

    Q_ASSERT(trVector.size()==1);
    const WalletTransaction & tx = trVector[0];

    // Continue with outputs
    QVector< WalletOutput > outputResult;
    parseOutputs(events, // in
                 account, height, outputResult); // out

    QVector<QString> messages;
    parseMessages(events, messages);

    wallet713->setTransactionById( true,  account, height, tx, outputResult, messages );

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

    parseTransactions(events, // in
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

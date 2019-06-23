#include "TaskTransaction.h"
#include "../mwc713.h"
#include <QDebug>

namespace wallet {

struct TransactionIdxLayout {
    int posId = -1;
    int posType = -1;
    int posTxid = -1;
    int posAddress = -1;
    int posCrTime = -1;
    int posConf = -1;
    int posConfTime = -1;
    int posNetDiff = -1;
    int posProof = -1;

    bool isDefined() const {
        return posId>=0 && posType>0 && posTxid>0 && posAddress>0 && posCrTime>0 &&
                            posConf>0 && posConfTime>0 && posNetDiff>0 && posProof>0;
    }
};

bool parseTransactionLine( QString str, const TransactionIdxLayout & tl,
                           WalletTransaction & trans) {

    QString strId = util::getSubString(str, tl.posId, tl.posType);
    QString strType = util::getSubString(str, tl.posType, tl.posTxid);
    QString strTxid = util::getSubString(str, tl.posTxid, tl.posAddress);
    QString strAddress = util::getSubString(str, tl.posAddress, tl.posCrTime);
    QString strCrTime = util::getSubString(str, tl.posCrTime, tl.posConf);
    QString strConf = util::getSubString(str, tl.posConf, tl.posConfTime);
    QString strConfTime = util::getSubString(str, tl.posConfTime, tl.posNetDiff);
    QString strNetDiff = util::getSubString(str, tl.posNetDiff, tl.posProof);
    QString strProof = util::getSubString(str, tl.posProof, str.length());

    if (strId.length() == 0)
        return false;

    bool ok = false;
    long id = -1;
    id = strId.toLong(&ok);
    if (!ok || id < 0)
        return false;

    WalletTransaction::TRANSACTION_TYPE tansType = WalletTransaction::TRANSACTION_TYPE::NONE;
    if (strType.startsWith("Sent"))
        tansType = WalletTransaction::TRANSACTION_TYPE::SEND;
    else if (strType.startsWith("Received"))
        tansType = WalletTransaction::TRANSACTION_TYPE::RECIEVE;
    else
        return false;

    if (strTxid.isEmpty() || strCrTime.isEmpty())
        return false;

    bool conf = strConf.startsWith("yes", Qt::CaseInsensitive);

    QPair<bool, long> net = util::one2nano(strNetDiff);
    if ( !net.first )
        return false;

    bool proof = strProof.startsWith("yes", Qt::CaseInsensitive);

    trans.setData(id, tansType,
                  strTxid,
                  strAddress,
                  strCrTime,
                  conf,
                  strConfTime,
                  net.second,
                  proof);
    return true;
}


bool TaskTransactions::processTask(const QVector<WEvent> & events) {
    // We are processing transactions outptu mostly as a raw data

    int curEvt = 0;

    QString account;
    long  height = -1;

    for ( ; curEvt < events.size(); curEvt++ ) {
        if (events[curEvt].event == WALLET_EVENTS::S_TRANSACTION_LOG ) {
            QStringList l = events[curEvt].message.split('|');
            Q_ASSERT(l.size()==2);
            account = l[0];
            height = l[1].toInt();
            break;
        }
    }

    // positions for the columns. Note, the columns and the order are hardcoded and come from wmc713 data!!!

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
                tl.posConfTime = str.indexOf("Confirmation Time", tl.posConf);
                tl.posNetDiff = str.indexOf("Net", tl.posConfTime);
                tl.posProof = str.indexOf("Proof?", tl.posNetDiff);

                if ( tl.isDefined() ) {
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
            if (str.startsWith("=============="))
                break;
        }
    }

    QMap< long, WalletTransaction > transactions;

    // Processing transactions
    long lastTransId = -1;
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

    QVector<WalletTransaction> trVector;
    for ( WalletTransaction & trItem : transactions )
        trVector.push_back( trItem );

    wallet713->setTransactions( account, height, trVector );
    return true;
}


}

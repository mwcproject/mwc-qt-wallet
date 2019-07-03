#include "wallet/wallet.h"
#include <QDataStream>
#include <QDateTime>
#include "../core/global.h"

namespace wallet {


QString toString(InitWalletStatus status) {
    switch (status) {
        case NONE:          return "NONE";
        case NEED_PASSWORD: return "NEED_PASSWORD";
        case NEED_INIT:     return "NEED_INIT";
        case WRONG_PASSWORD:return "WRONG_PASSWORD";
        case READY:         return "READY";
        default:            Q_ASSERT(false); return "Unknown";
    }
}

// To debug string
QString WalletNotificationMessages::toString() const {
    QString levelStr;
    switch (level) {
        case ERROR:     levelStr="ERROR"; break;
        case WARNING:   levelStr="WARNING"; break;
        case INFO:      levelStr="INFO"; break;
        case DEBUG:     levelStr="DEBUG"; break;
        default:        Q_ASSERT(false);
    }

    return ( "NotifMsg(level=" + levelStr + ", message="+message + ")" );
}

void AccountInfo::setData(QString account,
                        int64_t _total,
                        int64_t _awaitingConfirmation,
                        int64_t _lockedByPrevTransaction,
                        int64_t _currentlySpendable,
                        int64_t _mwcNodeHeight,
                        bool _mwcServerBroken)
{
    accountName = account;
    total = _total;
    awaitingConfirmation = _awaitingConfirmation;
    lockedByPrevTransaction = _lockedByPrevTransaction;
    currentlySpendable = _currentlySpendable;
    mwcNodeHeight = _mwcNodeHeight;
    mwcServerBroken = _mwcServerBroken;
}

QString AccountInfo::getLongAccountName() const {
    return accountName + "  Total: " + util::nano2one(total) + "mwc  " +
                "Spendable: " + util::nano2one(currentlySpendable) + "  Locked: " +
                util::nano2one(lockedByPrevTransaction) +
                "  Awaiting Confirmation: " + util::nano2one(awaitingConfirmation);
}

// return true is this account can be concidered as deleted
bool AccountInfo::isDeleted() const {
    return  accountName.startsWith( mwc::DEL_ACCONT_PREFIX ) &&
            total == 0 && awaitingConfirmation==0 && lockedByPrevTransaction==0 && currentlySpendable==0;
}


void WalletConfig::saveData( QDataStream & out) const {
    out << 0x32189;

    out << dataPath;
    out << mwcboxDomain;
    out << mwcboxPort;
    out << mwcNodeURI;
    out << mwcNodeSecret;
}

bool WalletConfig::loadData( QDataStream & in) {
    int id = 0;
    in >> id;
    if (id!=0x32189)
        return false;

    in >> dataPath;
    in >> mwcboxDomain;
    in >> mwcboxPort;
    in >> mwcNodeURI;
    in >> mwcNodeSecret;
    return true;
}

void NodeStatus::setData(int   _connection,
                            QString _tip,
                            int64_t _height,
                            int64_t _total_difficulty)
{
    connection = _connection;
    tip = _tip;
    height = _height;
    total_difficulty = _total_difficulty;
}


void WalletTransaction::setData(int64_t _txIdx,
    uint    _transactionType,
    QString _txid,
    QString _address,
    QString _creationTime,
    bool    _confirmed,
    QString _confirmationTime,
    int64_t    _coinNano,
    bool    _proof)
{
    txIdx = _txIdx;
    transactionType = _transactionType;
    txid = _txid;
    address = _address;
    creationTime = util::mwc713time2ThisTime(_creationTime);
    confirmed = _confirmed;
    confirmationTime = util::mwc713time2ThisTime(_confirmationTime);
    coinNano = _coinNano;
    proof = _proof;
}

// return transaction age (time interval from creation moment) in Seconds.
int64_t WalletTransaction::calculateTransactionAge( const QDateTime & current ) const {
    // Example: 2019-06-22 05:44:53
    QDateTime setTime = QDateTime::fromString (creationTime, mwc::DATETIME_TEMPLATE_THIS );
//    setTime.setOffsetFromUtc(0);
    return setTime.secsTo(current);
}



void WalletOutput::setData(QString _outputCommitment,
        int64_t    _MMRIndex,
        int64_t    _lockHeight,
        bool    _lockedUntil,
        STATUS  _status,
        bool    _coinbase,
        int64_t    _numOfConfirms,
        int64_t    _valueNano,
        int64_t    _txIdx)
{
    outputCommitment = _outputCommitment;
    MMRIndex = _MMRIndex;
    lockHeight = _lockHeight;
    lockedUntil = _lockedUntil;
    status = _status;
    coinbase = _coinbase;
    numOfConfirms = _numOfConfirms;
    valueNano = _valueNano;
    txIdx = _txIdx;
}

// return status value as a string
QString WalletOutput::getStatusStr() const {
    switch (status) {
    case Unconfirmed:
        return "Unconfirmed";
    case Confirmed:
        return "Confirmed";
    default:
        Q_ASSERT(false);
        return "Unknown";
    }
}


void WalletContact::setData(QString _name,
    QString _address)
{
    name = _name;
    address = _address;
}

void WalletContact::saveData( QDataStream & out) const {
    out << 0x89365;
    out << name;
    out << address;
}

bool WalletContact::loadData( QDataStream & in) {
    int id = 0;
    in >> id;
    if (id!=0x89365)
        return false;

    in >> name;
    in >> address;
    return true;
}


void WalletUtxoSignature::setData(int64_t _coinNano, // Output amount
        QString _messageHash,
        QString _pubKeyCompressed,
        QString _messageSignature)
{
    coinNano = _coinNano;
    messageHash = _messageHash;
    pubKeyCompressed = _pubKeyCompressed;
    messageSignature = _messageSignature;
}


Wallet::Wallet()
{
}

Wallet::~Wallet()
{
}

}

#include "wallet/wallet.h"
#include <QDataStream>
#include <QDateTime>

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
                        long _total,
                        long _awaitingConfirmation,
                        long _lockedByPrevTransaction,
                        long _currentlySpendable,
                        long _mwcNodeHeight,
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
                            long _height,
                            long _total_difficulty)
{
    connection = _connection;
    tip = _tip;
    height = _height;
    total_difficulty = _total_difficulty;
}


void WalletTransaction::setData(long _txIdx,
    uint    _transactionType,
    QString _txid,
    QString _address,
    QString _creationTime,
    bool    _confirmed,
    QString _confirmationTime,
    long    _coinNano,
    bool    _proof)
{
    txIdx = _txIdx;
    transactionType = _transactionType;
    txid = _txid;
    address = _address;
    creationTime = _creationTime;
    confirmed = _confirmed;
    confirmationTime = _confirmationTime;
    coinNano = _coinNano;
    proof = _proof;
}

// return transaction age (time interval from creation moment) in Seconds.
long WalletTransaction::calculateTransactionAge( const QDateTime & current ) const {
    // Example: 2019-06-22 05:44:53
    QDateTime setTime = QDateTime::fromString (creationTime, "yyyy-MM-dd hh:mm:ss" );
    setTime.setOffsetFromUtc(0);
    return setTime.secsTo(current);
}



void WalletOutput::setData(QString _outputCommitment,
        long    _MMRIndex,
        long    _lockHeight,
        bool    _lockedUntil,
        STATUS  _status,
        bool    _coinbase,
        long    _numOfConfirms,
        long    _valueNano,
        long    _txIdx)
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


void WalletUtxoSignature::setData(long _coinNano, // Output amount
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

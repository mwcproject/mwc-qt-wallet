#include "wallet/wallet.h"
#include <QDataStream>

namespace wallet {


void WalletInfo::setData(QString account,
                        long _total,
                        long _awaitingConfirmation,
                        long _lockedByPrevTransaction,
                        long _currentlySpendable)
{
    accountName = account;
    total = _total;
    awaitingConfirmation = _awaitingConfirmation;
    lockedByPrevTransaction = _lockedByPrevTransaction;
    currentlySpendable = _currentlySpendable;
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


void WalletProofInfo::setDataSuccess(long _coinsNano,
                            QString _fromAddress,
                            QString _toAddress,
                            QString _output,
                            QString _kernel)
{
    successed = true;
    errorMessage = "";
    coinsNano = _coinsNano;
    fromAddress = _fromAddress;
    toAddress = _toAddress;
    output = _output;
    kernel = _kernel;
}

void WalletProofInfo::setDataFailure(QString _errorMessage) {
    successed = false;
    errorMessage = _errorMessage;
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


void WalletOutput::setData(QString _outputCommitment,
        long    _MMRIndex,
        long    _mlockHeight,
        bool    _lockedUntil,
        STATUS  _status,
        bool    _coinbase,
        long    _numOfConfirms,
        long    _valueNano,
        long    _txIdx)
{
    outputCommitment = _outputCommitment;
    MMRIndex = _MMRIndex;
    mlockHeight = _mlockHeight;
    lockedUntil = _lockedUntil;
    status = _status;
    coinbase = _coinbase;
    numOfConfirms = _numOfConfirms;
    valueNano = _valueNano;
    txIdx = _txIdx;
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

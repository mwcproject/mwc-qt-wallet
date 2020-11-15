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

#include "wallet/wallet.h"
#include <QDataStream>
#include <QDateTime>
#include "../core/global.h"
#include "../core/Config.h"
#include <QDir>
#include "../util/Files.h"
#include "../util/Process.h"
#include "../core/WndManager.h"
#include "../core/appcontext.h"
#include <QJsonObject>
#include <QJsonDocument>

namespace wallet {

void AccountInfo::setData(QString account,
                        int64_t _total,
                        int64_t _awaitingConfirmation,
                        int64_t _lockedByPrevTransaction,
                        int64_t _currentlySpendable,
                        int64_t _height,
                        bool _mwcServerBroken)
{
    accountName = account;
    total = _total;
    awaitingConfirmation = _awaitingConfirmation;
    lockedByPrevTransaction = _lockedByPrevTransaction;
    currentlySpendable = _currentlySpendable;
    height = _height;
    mwcServerBroken = _mwcServerBroken;
}

QString AccountInfo::getLongAccountName() const {
    return accountName + "  Total: " + util::nano2one(total) + " MWC  " +
                "Spendable: " + util::nano2one(currentlySpendable) + "  Locked: " +
                util::nano2one(lockedByPrevTransaction) +
                "  Unconfirmed: " + util::nano2one(awaitingConfirmation);
}

QString AccountInfo::getSpendableAccountName() const {
    return  util::expandStrR(accountName, 15) +
           "   Available: " + util::nano2one(currentlySpendable) + " MWC";

}


// return true is this account can be concidered as deleted
bool AccountInfo::isDeleted() const {
    return  accountName.startsWith( mwc::DEL_ACCONT_PREFIX ) &&
            total == 0 && awaitingConfirmation==0 && lockedByPrevTransaction==0 && currentlySpendable==0;
}


// Debug/Log printing
QString AccountInfo::toString() const {
    return "AccountInfo(acc=" + accountName + ", total=" + util::nano2one(total) +
               " spend=" + util::nano2one(currentlySpendable) + " locked=" +
               util::nano2one(lockedByPrevTransaction) +
               " awaiting=" + util::nano2one(awaitingConfirmation) + ")";
}


void MwcNodeConnection::saveData(QDataStream & out) const {
    int id = 0x4355a2;
    out << id;
    out << (int)connectionType;
    out << mwcNodeURI;
    out << mwcNodeSecret;
    out << localNodeDataPath;
}

bool MwcNodeConnection::loadData(QDataStream & in) {
    int id = 0;
    in >> id;
    if (id<0x4355a1 || id>0x4355a2)
        return false;

    int conType = (int)NODE_CONNECTION_TYPE::CLOUD;
    in >> conType;
    connectionType = (NODE_CONNECTION_TYPE) conType;
    in >> mwcNodeURI;
    in >> mwcNodeSecret;

    if (id>=0x4355a2)
        in >> localNodeDataPath;

    return true;
}

QString MwcNodeConnection::toJson() {
    QJsonObject obj;
    obj.insert("connectionType", int(connectionType) );
    obj.insert("localNodeDataPath", localNodeDataPath );
    obj.insert("mwcNodeURI", mwcNodeURI );
    obj.insert("mwcNodeSecret", mwcNodeSecret );

    return QJsonDocument(obj).toJson(QJsonDocument::JsonFormat::Compact);

}
// static
MwcNodeConnection MwcNodeConnection::fromJson(const QString & str) {
    QJsonParseError error;
    QJsonDocument   jsonDoc = QJsonDocument::fromJson(str.toUtf8(), &error);
    // Internal data, no error expected
    Q_ASSERT( error.error == QJsonParseError::NoError );
    Q_ASSERT(jsonDoc.isObject());
    QJsonObject obj = jsonDoc.object();

    MwcNodeConnection res;
    res.setData( NODE_CONNECTION_TYPE(obj.value("connectionType").toInt()),
                obj.value("localNodeDataPath").toString(),
                obj.value("mwcNodeURI").toString(),
                obj.value("mwcNodeSecret").toString() );
    return res;
}

void MwcNodeConnection::setData(NODE_CONNECTION_TYPE _connectionType, const QString & _localNodeDataPath,
             const QString & _mwcNodeURI,  const QString & _mwcNodeSecret) {
    connectionType = _connectionType;
    localNodeDataPath = _localNodeDataPath;
    mwcNodeURI = _mwcNodeURI;
    mwcNodeSecret = _mwcNodeSecret;
}


bool WalletConfig::operator == (const WalletConfig & other) const {
    bool ok = dataPath==other.dataPath &&
              mwcmqsDomainEx==other.mwcmqsDomainEx &&
              foreignApi==other.foreignApi;

    if (!ok)
        return ok;

    ok = foreignApiAddress == other.foreignApiAddress && foreignApiSecret == other.foreignApiSecret &&
            tlsCertificateFile == other.tlsCertificateFile && tlsCertificateKey == other.tlsCertificateKey;

    return ok;
}


WalletConfig & WalletConfig::setData(QString _network,
                            QString _dataPath,
                            QString _mwcmqsDomain,
                            bool    _foreignApi,
                            QString _foreignApiAddress,
                            QString _foreignApiSecret,
                            QString _tlsCertificateFile,
                            QString _tlsCertificateKey) {
    updateNetwork(_network);
    updateDataPath(_dataPath);
    setDataWalletCfg(_mwcmqsDomain);

    return setForeignApi(_foreignApi, _foreignApiAddress, _foreignApiSecret, _tlsCertificateFile, _tlsCertificateKey);
}

WalletConfig & WalletConfig::setForeignApi(bool _foreignApi,
                             QString _foreignApiAddress, QString _foreignApiSecret,
                             QString _tlsCertificateFile, QString _tlsCertificateKey) {
    foreignApi = _foreignApi;
    foreignApiAddress = _foreignApiAddress;
    foreignApiSecret = _foreignApiSecret;
    tlsCertificateFile = _tlsCertificateFile;
    tlsCertificateKey = _tlsCertificateKey;

    return * this;
}



WalletConfig & WalletConfig::setDataWalletCfg( QString _mwcmqsDomain) {
    mwcmqsDomainEx = _mwcmqsDomain;
    return *this;
}

QString WalletConfig::toString() const {
    return "network=" + network + "\n" +
            "dataPath=" + dataPath + "\n" +
            "mwcmqsDomainEx=" + mwcmqsDomainEx;
}

// Get MQ/MQS host name. Depend on current config
QString WalletConfig::getMwcMqHostNorm() const {
    return mwcmqsDomainEx;
}

// Get MQ/MQS host full name. Depend on current config
QString WalletConfig::getMwcMqHostFull() const {
    QString definedHost = getMwcMqHostNorm();
    if (!definedHost.isEmpty())
        return definedHost;

    return mwc::DEFAULT_HOST_MWC_MQS;
}

// Return empty if not found
//static
QVector<QString>  WalletConfig::readNetworkArchInstanceFromDataPath(QString configPath, core::AppContext * context) // local path as writen in config
{
    QString defaultNetwork = "Mainnet";
    if (config::isOnlineNode()) {
        if ( context->isOnlineNodeRunsMainNetwork() )
            defaultNetwork = "Mainnet";
        else
            defaultNetwork = "Floonet";
    }

    QVector<QString> res{defaultNetwork, util::getBuildArch(), configPath};

    QPair<bool,QString> path = ioutils::getAppDataPath( configPath );
    if (!path.first) {
        core::getWndManager()->messageTextDlg("Error", path.second);
        return res;
    }

    QStringList lns = util::readTextFile(path.second + QDir::separator() + "net.txt" );
    if (lns.isEmpty())
        return res;


    QString nw = lns[0];
    if (!nw.contains("net"))
        return res;

    res[0] = nw;

    // Architecture
    if (lns.size()>1)
        res[1] = lns[1];

    // Instance name
    if (lns.size()>2)
        res[2] = lns[2];

    return res;
}

//static
bool  WalletConfig::doesSeedExist(QString configPath) {
    QPair<bool,QString> path = ioutils::getAppDataPath( configPath, false );
    if (!path.first) {
        core::getWndManager()->messageTextDlg("Error", path.second);
        return false;
    }
    return QFile::exists( path.second + QDir::separator() + "wallet.seed" );
}

//static
void  WalletConfig::saveNetwork2DataPath(QString configPath, QString network, QString arch, QString instanceName) // Save the network into the data path
{
    QPair<bool,QString> path = ioutils::getAppDataPath( configPath );
    if (!path.first) {
        core::getWndManager()->messageTextDlg("Error", path.second);
        return;
    }
    util::writeTextFile(path.second + "/net.txt", {network, arch, instanceName} );
}

// initialize static csvHeaders
// the CSV headers must match the output from mwc713 for 'txs --show-full'
QString WalletTransaction::csvHeaders = "Id,Type,Shared Transaction Id,Address,Creation Time,TTL Cutoff Height,"
                                        "Confirmed?,Height,Confirmation Time,Num. Inputs,Num. Outputs,Amount Credited,"
                                        "Amount Debited,Fee,Net Difference,Payment Proof,Kernel";


void WalletTransaction::setData(int64_t _txIdx,
                                uint    _transactionType,
                                QString _txid,
                                QString _address,
                                QString _creationTime,
                                bool    _confirmed,
                                int64_t _ttlCutoffHeight,
                                int64_t _height,
                                QString _confirmationTime,
                                int     _numInputs,
                                int     _numOutputs,
                                int64_t _credited,
                                int64_t _debited,
                                int64_t _fee,
                                int64_t _coinNano,
                                bool    _proof,
                                QString _kernel)
{
    txIdx = _txIdx;
    transactionType = _transactionType;
    txid = _txid;
    address = _address;
    creationTime = util::mwc713time2ThisTime(_creationTime);
    confirmed = _confirmed;
    height = _height;
    confirmationTime = util::mwc713time2ThisTime(_confirmationTime);
    coinNano = _coinNano;
    proof = _proof;
    ttlCutoffHeight = _ttlCutoffHeight;
    numInputs = _numInputs;
    numOutputs = _numOutputs;
    credited = _credited;
    debited = _debited;
    fee = _fee;
    kernel = _kernel;
}

// return transaction age (time interval from creation moment) in Seconds.
int64_t WalletTransaction::calculateTransactionAge( const QDateTime & current ) const {
    // Example: 2019-06-22 05:44:53
    QDateTime setTime = QDateTime::fromString (creationTime, mwc::DATETIME_TEMPLATE_THIS );
//    setTime.setOffsetFromUtc(0);
    return setTime.secsTo(current);
}

QString WalletTransaction::toStringCSV() const {
    QString separator = ",";
    // always enclose the type string in quotes as it could contain a comma
    QString txTypeStr = "\"" + getTypeAsStr() + "\"";
    QString csvStr = QString::number(txIdx) + separator +       // Id
                      txTypeStr + separator +                   // Type
                      txid + separator +                        // Shared Transaction Id
                      address + separator +                     // Address
                      creationTime + separator +                // Creation Time
                      QString::number(ttlCutoffHeight) + separator + // TTL Cutoff Height
                      (confirmed ? "YES" : "NO") + separator +  // Confirmed?
                      QString::number(height) + separator +     // height
                      confirmationTime + separator +            // Confirmation Time
                      QString::number(numInputs) + separator +  // Num. Inputs
                      QString::number(numOutputs) + separator + // Num. Outputs
                      util::nano2one(credited) + separator +    // Amount Credited
                      util::nano2one(debited) + separator +     // Amount Debited
                      util::nano2one(fee) + separator +         // Fee
                      util::nano2one(coinNano) + separator +    // Net Difference
                      (proof ? "yes" : "no") + separator +      // Payment Proof
                      kernel;                                   // Kernel
    return csvStr;
}

QString WalletTransaction::toJson() const {
    QJsonObject obj;
    obj.insert("txIdx", QString::number(txIdx) );
    obj.insert("transactionType", int(transactionType) );
    obj.insert("txid", txid );
    obj.insert("address", address);
    obj.insert("creationTime", creationTime);
    obj.insert("ttlCutoffHeight", QString::number(ttlCutoffHeight) );
    obj.insert("confirmed", confirmed);
    obj.insert("height", QString::number(height) );
    obj.insert("confirmationTime", confirmationTime);
    obj.insert("numInputs", numInputs);
    obj.insert("numOutputs", numOutputs);
    obj.insert("credited", QString::number(credited));
    obj.insert("debited", QString::number(debited));
    obj.insert("fee", QString::number(fee));
    obj.insert("coinNano", QString::number(coinNano) );
    obj.insert("proof", proof);
    obj.insert("kernel", kernel);

    return QJsonDocument(obj).toJson(QJsonDocument::JsonFormat::Compact);
}
//static
WalletTransaction WalletTransaction::fromJson(QString str) {
    QJsonParseError error;
    QJsonDocument   jsonDoc = QJsonDocument::fromJson(str.toUtf8(), &error);
    // Internal data, no error expected
    Q_ASSERT( error.error == QJsonParseError::NoError );
    Q_ASSERT(jsonDoc.isObject());
    QJsonObject obj = jsonDoc.object();

    WalletTransaction res;
    res.setData(obj.value("txIdx").toString().toLongLong(),
            uint(obj.value("transactionType").toInt()),
            obj.value("txid").toString(),
            obj.value("address").toString(),
            obj.value("creationTime").toString(),
            obj.value("confirmed").toBool(),
            obj.value("ttlCutoffHeight").toString().toLongLong(),
            obj.value("height").toString().toLongLong(),
            obj.value("confirmationTime").toString(),
            obj.value("numInputs").toInt(),
            obj.value("numOutputs").toInt(),
            obj.value("credited").toString().toLongLong(),
            obj.value("debited").toString().toLongLong(),
            obj.value("fee").toString().toLongLong(),
            obj.value("coinNano").toString().toLongLong(),
            obj.value("proof").toBool(),
            obj.value("kernel").toString());
    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  WalletOutput

void WalletOutput::setData(QString _outputCommitment,
        QString     _MMRIndex,
        QString     _blockHeight,
        QString     _lockedUntil,
        QString     _status,
        bool        _coinbase,
        QString     _numOfConfirms,
        int64_t     _valueNano,
        int64_t     _txIdx)
{
    outputCommitment = _outputCommitment;
    MMRIndex = _MMRIndex;
    blockHeight = _blockHeight;
    lockedUntil = _lockedUntil;
    status = _status;
    coinbase = _coinbase;
    numOfConfirms = _numOfConfirms;
    valueNano = _valueNano;
    txIdx = _txIdx;
}

QString WalletOutput::toString() const {
    return  "Output(" + outputCommitment + ", MMR=" + MMRIndex + ", Height=" + blockHeight + ", Locked=" + lockedUntil + ", status=" +
            status + ", coinbase=" + (coinbase?"true":"false") + ", confirms=" + numOfConfirms, ", value=" + QString::number(valueNano) + ", txIdx=" + txIdx + ")";
}

QString WalletOutput::toJson() const {
    QJsonObject obj;
    obj.insert("outputCommitment", outputCommitment);
    obj.insert("MMRIndex", MMRIndex);
    obj.insert("blockHeight", blockHeight);
    obj.insert("lockedUntil", lockedUntil);
    obj.insert("status", status);
    obj.insert("coinbase", coinbase);
    obj.insert("numOfConfirms", numOfConfirms);
    obj.insert("valueNano", QString::number(valueNano) );
    obj.insert("txIdx", QString::number(txIdx) );
    obj.insert("weight", weight);

    return QJsonDocument(obj).toJson(QJsonDocument::JsonFormat::Compact);
}

//static
WalletOutput WalletOutput::fromJson(QString str) {
    QJsonParseError error;
    QJsonDocument   jsonDoc = QJsonDocument::fromJson(str.toUtf8(), &error);
    // Internal data, no error expected
    Q_ASSERT( error.error == QJsonParseError::NoError );
    Q_ASSERT(jsonDoc.isObject());
    QJsonObject obj = jsonDoc.object();

    WalletOutput res;
    res.setData(obj.value("outputCommitment").toString(),
                obj.value("MMRIndex").toString(),
                obj.value("blockHeight").toString(),
                obj.value("lockedUntil").toString(),
                obj.value("status").toString(),
                obj.value("coinbase").toBool(),
                obj.value("numOfConfirms").toString(),
                obj.value("valueNano").toString().toLongLong(),
                obj.value("txIdx").toString().toLongLong());
    return res;
}


///////////////////////////////////////////////////////////////////////////////////////////
//  WalletUtxoSignature

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

/////////////////////////////////////////////////////////////////////////////////
// SwapInfo

void SwapInfo::setData( QString _mwcAmount, QString _secondaryAmount, QString _secondaryCurrency,
              QString _swapId, int64_t _startTime, QString _stateCmd, QString _state, QString _action, int64_t _expiration,
              bool _isSeller, QString _secondaryAddress, QString _lastProcessError ) {
    mwcAmount = _mwcAmount;
    secondaryAmount = _secondaryAmount;
    secondaryCurrency = _secondaryCurrency;
    swapId = _swapId;
    startTime = _startTime;
    stateCmd = _stateCmd;
    state = _state;
    action = _action;
    expiration = _expiration;
    isSeller = _isSeller;
    secondaryAddress = _secondaryAddress;
    lastProcessError = _lastProcessError;
}

/////////////////////////////////////////////////////////////////////////////////
// SwapTradeInfo

void SwapTradeInfo::setData( QString _swapId, bool _isSeller, double _mwcAmount, double _secondaryAmount,
              QString _secondaryCurrency,  QString _secondaryAddress, double _secondaryFee,
              QString _secondaryFeeUnits, int _mwcConfirmations, int _secondaryConfirmations,
              int _messageExchangeTimeLimit, int _redeemTimeLimit, bool _sellerLockingFirst,
              int _mwcLockHeight, int64_t _mwcLockTime, int64_t _secondaryLockTime,
              QString _communicationMethod, QString _communicationAddress, QString _electrumNodeUri ) {

    swapId = _swapId;
    isSeller = _isSeller;
    mwcAmount = _mwcAmount;
    secondaryAmount = _secondaryAmount;
    secondaryCurrency = _secondaryCurrency;
    secondaryAddress = _secondaryAddress;
    secondaryFee = _secondaryFee;
    secondaryFeeUnits = _secondaryFeeUnits;
    mwcConfirmations = _mwcConfirmations;
    secondaryConfirmations = _secondaryConfirmations;
    messageExchangeTimeLimit = _messageExchangeTimeLimit;
    redeemTimeLimit = _redeemTimeLimit;
    sellerLockingFirst = _sellerLockingFirst;
    mwcLockHeight = _mwcLockHeight;
    mwcLockTime = _mwcLockTime;
    secondaryLockTime = _secondaryLockTime;
    communicationMethod = _communicationMethod;
    communicationAddress = _communicationAddress;
    electrumNodeUri = _electrumNodeUri;
}

////////////////////////////////////////////////////////////////////

void SwapExecutionPlanRecord::setData( bool _active, int64_t _end_time, QString _name ) {
    active = _active;
    end_time = _end_time;
    name = _name;
}

///////////////////////////////////////////////////////////////////
void SwapJournalMessage::setData( QString _message, int64_t _time ) {
    message = _message;
    time = _time;
}

//////////////////////////////////////////////////////////////////
Wallet::Wallet()
{
}

Wallet::~Wallet()
{
}

}

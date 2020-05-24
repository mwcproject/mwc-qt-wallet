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
#include "../control/messagebox.h"

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
    return accountName + "  Total: " + util::nano2one(total) + "mwc  " +
                "Spendable: " + util::nano2one(currentlySpendable) + "  Locked: " +
                util::nano2one(lockedByPrevTransaction) +
                "  Awaiting Confirmation: " + util::nano2one(awaitingConfirmation);
}

QString AccountInfo::getSpendableAccountName() const {
    return  util::expandStrR(accountName, 15) +
           "   Available: " + util::nano2one(currentlySpendable) + " mwc";

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

bool WalletConfig::operator == (const WalletConfig & other) const {
    bool ok = dataPath==other.dataPath &&
              mwcmqDomainEx==other.mwcmqDomainEx && mwcmqsDomainEx==other.mwcmqsDomainEx &&
              keyBasePath==other.keyBasePath && foreignApi==other.foreignApi;

    if (!ok)
        return ok;

    ok = foreignApiAddress == other.foreignApiAddress && foreignApiSecret == other.foreignApiSecret &&
            tlsCertificateFile == other.tlsCertificateFile && tlsCertificateKey == other.tlsCertificateKey;

    return ok;
}


WalletConfig & WalletConfig::setData(QString _network,
                            QString _dataPath,
                            QString _mwcmqDomain,
                            QString _mwcmqsDomain,
                            QString _keyBasePath,
                            bool    _foreignApi,
                            QString _foreignApiAddress,
                            QString _foreignApiSecret,
                            QString _tlsCertificateFile,
                            QString _tlsCertificateKey) {

    setDataWalletCfg(_network,_dataPath,_mwcmqDomain,_mwcmqsDomain,_keyBasePath);

    foreignApi = _foreignApi;
    foreignApiAddress = _foreignApiAddress;
    foreignApiSecret = _foreignApiSecret;
    tlsCertificateFile = _tlsCertificateFile;
    tlsCertificateKey = _tlsCertificateKey;

    return * this;
}

WalletConfig & WalletConfig::setDataWalletCfg(QString _network,
                                QString _dataPath,
                                QString _mwcmqDomain,
                                QString _mwcmqsDomain,
                                QString _keyBasePath)
{
    network  = _network;
    dataPath = _dataPath;
    mwcmqDomainEx = _mwcmqDomain;
    mwcmqsDomainEx = _mwcmqsDomain;
    keyBasePath = _keyBasePath;

    return *this;
}


QString WalletConfig::toString() const {
    return "network=" + network + "\n" +
            "dataPath=" + dataPath + "\n" +
            "mwcmqDomainEx=" + mwcmqDomainEx + "\n" +
            "mwcmqsDomainEx=" + mwcmqsDomainEx + "\n" +
            "keyBasePath=" + keyBasePath;
}


// Get MQ/MQS host name. Depend on current config
QString WalletConfig::getMwcMqHostNorm() const {
    return config::getUseMwcMqS() ? mwcmqsDomainEx : mwcmqDomainEx;
}

// Get MQ/MQS host full name. Depend on current config
QString WalletConfig::getMwcMqHostFull() const {
    QString definedHost = getMwcMqHostNorm();
    if (!definedHost.isEmpty())
        return definedHost;

    return config::getUseMwcMqS() ? mwc::DEFAULT_HOST_MWC_MQS : mwc::DEFAULT_HOST_MWC_MQ;
}


// Return empty if not found
//static
QPair<QString,QString> WalletConfig::readNetworkArchFromDataPath(QString configPath) // local path as writen in config
{
    QPair<QString,QString> res("", util::getBuildArch() );

    QPair<bool,QString> path = ioutils::getAppDataPath( configPath );
    if (!path.first) {
        control::MessageBox::messageText(nullptr, "Error", path.second);
        return res;
    }

    QStringList lns = util::readTextFile(path.second + "/net.txt" );
    if (lns.isEmpty())
        return res;


    QString nw = lns[0];
    if (!nw.contains("net"))
        return res;

    res.first = nw;

    if (lns.size()>1)
        res.second = lns[1];

    return res;
}

//static
bool  WalletConfig::doesSeedExist(QString configPath) {
    QPair<bool,QString> path = ioutils::getAppDataPath( configPath );
    if (!path.first) {
        control::MessageBox::messageText(nullptr, "Error", path.second);
        return false;
    }
    return QFile::exists( path.second + "/" + "wallet.seed" );
}

//static
void  WalletConfig::saveNetwork2DataPath(QString configPath, QString network, QString arch) // Save the network into the data path
{
    QPair<bool,QString> path = ioutils::getAppDataPath( configPath );
    if (!path.first) {
        control::MessageBox::messageText(nullptr, "Error", path.second);
        return;
    }
    util::writeTextFile(path.second + "/net.txt", {network, arch} );
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

QString WalletTransaction::toStringCSV() {
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


Wallet::Wallet()
{
}

Wallet::~Wallet()
{
}

}

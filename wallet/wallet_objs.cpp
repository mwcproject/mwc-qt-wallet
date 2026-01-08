// Copyright 2015 The MWC Developers
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

#include "wallet_objs.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <qjsondocument.h>
#include <QJsonObject>

#include "core/appcontext.h"
#include "core/Config.h"
#include "core/global.h"
#include "core/WndManager.h"
#include "util/Files.h"
#include "util/ioutils.h"
#include "util/Process.h"
#include "util/stringutils.h"


namespace wallet {

static QJsonObject str2json(const QString & jsonStr) {
    QJsonParseError error;
    QJsonDocument   jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    // It is internal data, no errors expected
    Q_ASSERT(error.error == QJsonParseError::NoError);
    Q_ASSERT(jsonDoc.isObject());

    return jsonDoc.object();
}


//////////////////////////////////////////////////////////
//  AccountInfo

void AccountInfo::setData(QString account,
                        QString path,
                        int64_t _total,
                        int64_t _awaitingConfirmation,
                        int64_t _lockedByPrevTransaction,
                        int64_t _currentlySpendable,
                        int64_t _height)
{
    accountName = account;
    accountPath = path;
    total = _total;
    awaitingConfirmation = _awaitingConfirmation;
    lockedByPrevTransaction = _lockedByPrevTransaction;
    currentlySpendable = _currentlySpendable;
    height = _height;
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

///////////////////////////////////////////////////////////////////////////
//  WalletConfig

bool WalletConfig::operator == (const WalletConfig & other) const {
    bool ok = dataPath==other.dataPath;
    return ok;
}


WalletConfig & WalletConfig::setData(QString _network,
                            QString _dataPath) {
    updateNetwork(_network);
    updateDataPath(_dataPath);

    return * this;
}


QString WalletConfig::toString() const {
    return "network=" + network + "\n" +
            "dataPath=" + dataPath;
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


//////////////////////////////////////////////////////////////
// WalletTransaction

// initialize static csvHeaders
// the CSV headers must match the output from mwc713 for 'txs --show-full'
QString WalletTransaction::csvHeaders = "Id,Type,Shared Transaction Id,Address,Creation Time,TTL Cutoff Height,"
                                        "Confirmed?,Height,Confirmation Time,Num. Inputs,Num. Outputs,Amount Credited,"
                                        "Amount Debited,Fee,Net Difference,Kernel";


void WalletTransaction::setData(int64_t _txIdx,
                                uint    _transactionType,
                                QString _txid,
                                QString _address,
                                QString _creationTime,
                                bool    _confirmed,
                                int64_t _ttlCutoffHeight,
                                int64_t _output_height,
                                int64_t _chain_height,
                                QString _confirmationTime,
                                int     _numInputs,
                                int     _numOutputs,
                                int64_t _credited,
                                int64_t _debited,
                                int64_t _fee,
                                int64_t _coinNano,
                                QString _kernel,
                                QVector<QString> _inputs,
                                QVector<QString> _outputs,
                                QVector<WalletTransactionMessage> _messages)
{
    txIdx = _txIdx;
    transactionType = _transactionType;
    txid = _txid;
    address = _address;
    creationTime = util::utcTime2LocalTime(_creationTime);
    confirmed = _confirmed;
    output_height = _output_height;
    chain_height = _chain_height;
    confirmationTime = util::utcTime2LocalTime(_confirmationTime);
    coinNano = _coinNano;
    ttlCutoffHeight = _ttlCutoffHeight;
    numInputs = _numInputs;
    numOutputs = _numOutputs;
    credited = _credited;
    debited = _debited;
    fee = _fee;
    kernel = _kernel;
    inputs = _inputs;
    outputs = _outputs;
    messages = _messages;
}

// return transaction age (time interval from creation moment) in Seconds.
int64_t WalletTransaction::calculateTransactionAge( const QDateTime & current ) const {
    // Example: 2019-06-22 05:44:53
    QDateTime setTime = QDateTime::fromString (creationTime, mwc::DATETIME_TEMPLATE_THIS );
//    setTime.setOffsetFromUtc(0);
    return setTime.secsTo(current);
}

QString WalletTransaction::toStringCSV(const QStringList & extraData) const {
    QString separator = ",";
    // always enclose the type string in quotes as it could contain a comma

    // The non confirmed or cancelled transactions need to have balance 0.
    int64_t tx_credited = credited;
    int64_t tx_debited = debited;
    int64_t tx_fee = fee;
    int64_t tx_coinNano = coinNano;

    if (!confirmed) {
        tx_credited = tx_debited = tx_fee = tx_coinNano = 0;
    }

    QString txTypeStr = "\"" + getTypeAsStr() + "\"";
    QString csvStr = QString::number(txIdx) + separator +       // Id
                      txTypeStr + separator +                   // Type
                      txid + separator +                        // Shared Transaction Id
                      address + separator +                     // Address
                      creationTime + separator +                // Creation Time
                      QString::number(ttlCutoffHeight) + separator + // TTL Cutoff Height
                      (confirmed ? "YES" : "NO") + separator +  // Confirmed?
                      QString::number(output_height) + separator +     // height
                      confirmationTime + separator +            // Confirmation Time
                      QString::number(numInputs) + separator +  // Num. Inputs
                      QString::number(numOutputs) + separator + // Num. Outputs
                      util::nano2one(tx_credited) + separator +    // Amount Credited
                      util::nano2one(tx_debited) + separator +     // Amount Debited
                      util::nano2one(tx_fee) + separator +         // Fee
                      util::nano2one(tx_coinNano) + separator +    // Net Difference
                      kernel;                                   // Kernel
    for (const auto & dt : extraData)
        csvStr += separator + dt;

    return csvStr;
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
            status + ", coinbase=" + (coinbase?"true":"false") + ", confirms=" + numOfConfirms, ", value=" + QString::number(valueNano) + ", txIdx=" + QString::number(txIdx) + ")";
}

QJsonObject WalletOutput::toJson() const {
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
    return obj;
}

//static
WalletOutput WalletOutput::fromJson(const QJsonObject & obj) {
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
//  WalletTransaction

QJsonObject WalletTransaction::toJson() const {
    QJsonObject obj;
    obj["txIdx"] = txIdx;
    obj["transactionType"] = (int)transactionType;
    obj["txid"] = txid;
    obj["address"] = address;
    obj["creationTime"] = creationTime;
    obj["ttlCutoffHeight"] = ttlCutoffHeight;
    obj["confirmed"] = confirmed;
    obj["output_height"] = output_height;
    obj["chain_height"] = chain_height;
    obj["confirmationTime"] = confirmationTime;
    obj["numInputs"] = numInputs;
    obj["numOutputs"] = numOutputs;
    obj["credited"] = credited;
    obj["debited"] = debited;
    obj["fee"] = fee;
    obj["coinNano"] = coinNano;
    obj["kernel"] = kernel;

    QJsonArray inputsJson;
    for (const QString & in : inputs)
        inputsJson.push_back(in);
    obj["inputs"] = inputsJson;

    QJsonArray outputsJson;
    for (const QString & out : outputs)
        outputsJson.push_back(out);
    obj["outputs"] = outputsJson;

    QJsonArray messagesJson;
    for (const WalletTransactionMessage & msg : messages)
        messagesJson.push_back(msg.message);
    obj["messages"] = messagesJson;

    return obj;
}

WalletTransaction WalletTransaction::fromJson(const QJsonObject & obj) {
    WalletTransaction  res;
    res.txIdx = obj["txIdx"].toInteger();
    res.transactionType = obj["transactionType"].toInt();
    res.txid = obj["txid"].toString();
    res.address = obj["address"].toString();
    res.creationTime = obj["creationTime"].toString();
    res.ttlCutoffHeight = obj["ttlCutoffHeight"].toInteger();
    res.confirmed = obj["confirmed"].toBool();
    res.output_height = obj["output_height"].toInteger(-1);
    res.chain_height = obj["chain_height"].toInteger(-1);
    res.confirmationTime = obj["confirmationTime"].toString();
    res.numInputs = obj["numInputs"].toInt();
    res.numOutputs = obj["numOutputs"].toInt();
    res.credited = obj["credited"].toInteger();
    res.debited = obj["debited"].toInteger();
    res.fee = obj["fee"].toInteger();
    res.coinNano = obj["coinNano"].toInteger();
    res.kernel = obj["kernel"].toString();
    for (const auto & i : obj["inputs"].toArray() )
        res.inputs.push_back(i.toString());
    for (const auto & i : obj["outputs"].toArray() )
        res.outputs.push_back(i.toString());
    for (const auto & i : obj["messages"].toArray() )
        res.messages.push_back( WalletTransactionMessage(-3, i.toString()));

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
              QString _swapId, QString _tag, int64_t _startTime, QString _stateCmd, QString _state, QString _action, int64_t _expiration,
              bool _isSeller, QString _secondaryAddress, QString _lastProcessError ) {
    mwcAmount = _mwcAmount;
    secondaryAmount = _secondaryAmount;
    secondaryCurrency = _secondaryCurrency;
    swapId = _swapId;
    tag = _tag;
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

void SwapTradeInfo::setData( QString _swapId, QString _tag, bool _isSeller, double _mwcAmount, double _secondaryAmount,
              QString _secondaryCurrency,  QString _secondaryAddress, double _secondaryFee,
              QString _secondaryFeeUnits, int _mwcConfirmations, int _secondaryConfirmations,
              int _messageExchangeTimeLimit, int _redeemTimeLimit, bool _sellerLockingFirst,
              int _mwcLockHeight, int64_t _mwcLockTime, int64_t _secondaryLockTime,
              QString _communicationMethod, QString _communicationAddress, QString _electrumNodeUri ) {

    swapId = _swapId;
    tag = _tag;
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
// SwapExecutionPlanRecord

void SwapExecutionPlanRecord::setData( bool _active, int64_t _end_time, QString _name ) {
    active = _active;
    end_time = _end_time;
    name = _name;
}

///////////////////////////////////////////////////////////////////
//  SwapJournalMessage

void SwapJournalMessage::setData( QString _message, int64_t _time ) {
    message = _message;
    time = _time;
}

///////////////////////////////////////////////////////////////////
// IntegrityFees

IntegrityFees::IntegrityFees(QString jsonString) : IntegrityFees(str2json(jsonString)) {
}

IntegrityFees::IntegrityFees(QJsonObject json) {
    confirmed = json["confirmed"].toBool();
    expiration_height = json["expiration_height"].toInt();
    ask_fee = json["ask_fee"].toString().toLongLong();
    fee = json["fee"].toString().toLongLong();
    uuid = json["uuid"].toString();
}

QJsonObject IntegrityFees::toJSon() const {
    QJsonObject res {
            {"confirmed" , confirmed },
            {"expiration_height", int(expiration_height) }, // int for height is fine
            {"ask_fee", QString::number(ask_fee) },
            {"fee", QString::number(fee) },
            {"uuid" , uuid },
    };
    return res;
}

QString IntegrityFees::toJSonStr() const {
    QJsonDocument doc(toJSon());
    QString offerStrJson( doc.toJson(QJsonDocument::Compact));
    return offerStrJson;
}

//////////////////////////////////////////////////////////////////
//  BroadcastingMessage

BroadcastingMessage::BroadcastingMessage(const QJsonObject & json) {
    uuid = json["uuid"].toString();
    broadcasting_interval = json["broadcasting_interval"].toInt();
    fee = json["fee"].toString().toLongLong();
    message = json["message"].toString();
    published_time = json["published_time"].toInt();
}

//////////////////////////////////////////////////////////////////
//  MessagingStatus

MessagingStatus::MessagingStatus(const QJsonObject & json) {
    if ( json["gossippub_peers"].isNull() ) {
        connected = false;
    }
    else {
        connected = true;
        QJsonArray peers = json["gossippub_peers"].toArray();
        for ( int i=0; i<peers.size(); i++ ) {
            gossippub_peers.push_back( peers[i].toString() );
        }
    }

    received_messages = json["received_messages"].toInt();
    QJsonArray tps = json["topics"].toArray();
    for (int i=0; i<tps.size(); i++) {
        topics.push_back( tps[i].toString() );
    }

    QJsonArray myMsgs = json["broadcasting"].toArray();
    for (int i=0; i<myMsgs.size(); i++) {
        broadcasting.push_back( BroadcastingMessage(myMsgs[i].toObject()) );
    }
}

// Status for logs
QString MessagingStatus::toString() const {
    return "MessagingStatus(gossippub_peers=" + gossippub_peers.join(", ") +
        ", received_messages=" + QString::number(received_messages) +
        " topics=" + topics.join(",") +
        " broadcasting.size()=" + QString::number(broadcasting.size());
}

bool NodeStatus::isHealthy() const {
    return online && ((config::isColdWallet() || connections > 0) && totalDifficulty > 0
           && nodeHeight > peerHeight - mwc::NODE_HEIGHT_DIFF_LIMIT);
}

void MwcNodeConnectionDeprecated::saveData(QDataStream & out) const {
    int id = 0x4355a2;
    out << id;
    out << (int)1;
    out << QString("");
    out << QString("");
    out << QString("");
}
bool MwcNodeConnectionDeprecated::loadData(QDataStream & in) {
    int id = 0;
    in >> id;
    if (id<0x4355a1 || id>0x4355a2)
        return false;

    int conType = -1;
    in >> conType;
    QString tmp;
    in >> tmp;
    in >> tmp;

    if (id>=0x4355a2)
        in >> tmp;
    return true;
}

QJsonObject DecodedSlatepack::toJson() {
    QJsonObject res;
    res["slate"] = slate;
    res["content"] = content;
    res["sender"] = sender;
    res["recipient"] = recipient;
    res["error"] = error;
    return res;
}

DecodedSlatepack DecodedSlatepack::fromJson(QJsonObject obj) {
    DecodedSlatepack res;
    res.slate = obj["slate"].toObject();
    res.content = obj["content"].toString();
    res.sender = obj["sender"].toString();
    res.recipient = obj["recipient"].toString();
    res.error = obj["error"].toString();
    return res;
}

QJsonObject ViewWalletOutputResult::toJson() const {
    QJsonObject result;
    result["commit"] = commit;
    result["value"] = value;
    result["height"] = height;
    result["mmr_index"] = mmr_index;
    result["is_coinbase"] = is_coinbase;
    result["lock_height"] = lock_height;
    return result;
}

QJsonObject ViewWallet::toJson() const {
    QJsonObject result;
    result["rewind_hash"] = rewind_hash;
    result["total_balance"] = total_balance;
    result["last_pmmr_index"] = last_pmmr_index;

    QJsonArray output_result_json;
    for (const ViewWalletOutputResult & r : output_result) {
        output_result_json.append(r.toJson());
    }
    result["output_result"] = output_result_json;
    return result;
}

}

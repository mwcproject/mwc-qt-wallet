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

#ifndef MWC_QT_WALLET_WALLET_OBJS_H
#define MWC_QT_WALLET_WALLET_OBJS_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include "util/stringutils.h"

namespace core {
    class AppContext;
}

namespace wallet {
    struct Account {
        QString label;
        QString path; // Hex form of path.
    };

    struct AccountInfo {
        QString accountName = "default";
        QString accountPath = "";
        qint64 height = 0;
        // in nano coins
        qint64 total = 0;
        qint64 awaitingConfirmation = 0;
        qint64 lockedByPrevTransaction = 0;
        qint64 currentlySpendable = 0;

        void setData(QString account,
            QString path,
            qint64 total,
            qint64 awaitingConfirmation,
            qint64 lockedByPrevTransaction,
            qint64 currentlySpendable,
            qint64 height);

        QString getLongAccountName() const;
        QString getSpendableAccountName() const;

        bool canDelete() const {return total == 0 && awaitingConfirmation==0 && lockedByPrevTransaction==0 && currentlySpendable==0;}

        // return true is this account can be concidered as deleted
        bool isDeleted() const;

        bool isAwaitingSomething() const {return awaitingConfirmation>0 || lockedByPrevTransaction>0; }

        // Debug/Log printing
        QString toString() const;

        bool balancesAreEquals(const AccountInfo & accInfo) const;
    };

    struct AccountsInfo {
        QString tag;
        QStringList accounts;
        QString activeAccount;

        AccountsInfo() = default;
        AccountsInfo(const AccountsInfo &) = default;
        AccountsInfo(AccountsInfo &&) = default;

        AccountsInfo(QString tag, QString activeAccount, const QVector<AccountInfo> & accountInfo);
    };

// Wallet config
struct WalletConfig {
private:
    QString network; // Name of the network as wallet understand: "Floonet" or "Mainnet"
    QString dataPath;
public:
    WalletConfig() : network("Mainnet"), dataPath("Undefined") {}
    WalletConfig(const WalletConfig & other) = default;
    WalletConfig &operator = (const WalletConfig & other) = default;

    bool operator == (const WalletConfig & other) const;

    bool isDefined() const { return  dataPath!="Undefined"; }

    WalletConfig & setData(QString network,
                QString dataPath);

    void updateDataPath(const QString & path) {dataPath=path;}
    void updateNetwork(const QString & mw) { network=mw; }

    const QString & getDataPath() const {return dataPath;}
    const QString & getNetwork() const {return network;}

    // First [Network, Arch, InstanceName]
    static QVector<QString> readNetworkArchInstanceFromDataPath(QString configPath, core::AppContext * context); // local path as writen in config
    static void saveNetwork2DataPath(QString configPath, QString network, QString arch, QString instanceName); // Save the network into the data path

    static bool    doesSeedExist(QString configPath);

    QString toString() const;
};

struct WalletOutput {
    QString    outputCommitment;
    QString    MMRIndex;
    QString    blockHeight;
    QString    lockedUntil;
    QString    status;
    bool       coinbase = false;
    QString    numOfConfirms;
    qint64    valueNano = 0L;
    qint64    txIdx = -1;

    void setData(QString outputCommitment,
            QString     MMRIndex,
            QString     blockHeight,
            QString     lockedUntil,
            QString     status,
            bool        coinbase,
            QString     numOfConfirms,
            qint64     valueNano,
            qint64     txIdx);

    static WalletOutput create(QString outputCommitment,
                               QString     MMRIndex,
                               QString     blockHeight,
                               QString     lockedUntil,
                               QString     status,
                               bool        coinbase,
                               QString     numOfConfirms,
                               qint64     valueNano,
                               qint64     txIdx) {
        WalletOutput item;
        item.setData(outputCommitment,
                MMRIndex, blockHeight, lockedUntil,
                status, coinbase, numOfConfirms,
                valueNano, txIdx);
        return item;
    }

    QString toString() const;

    bool isValid() const {
        return !(outputCommitment.isEmpty() || status.isEmpty());
    }

    bool isUnspent() const {return status == "Unspent";}

    QJsonObject toJson() const;
    static WalletOutput fromJson(const QJsonObject & obj);
};

struct WalletTransactionMessage {
    int participant_id = -1;
    QString message;

    WalletTransactionMessage( int _participant_id, const QString & _message) : participant_id(_participant_id), message(_message) {}
    WalletTransactionMessage() = default;
    WalletTransactionMessage(const WalletTransactionMessage & ) = default;
    WalletTransactionMessage & operator = (const WalletTransactionMessage & ) = default;
};

struct WalletTransaction {
    enum TRANSACTION_TYPE { NONE=0, SEND=1, RECEIVE=2, COIN_BASE=4, CANCELLED=0x8000};

    qint64    txIdx = -1;
    uint    transactionType = TRANSACTION_TYPE::NONE;
    QString txid; // Full tx UUID
    QString address;
    QString creationTime;
    qint64 ttlCutoffHeight = -1;
    bool    confirmed = false;
    qint64 output_height = 0;
    qint64 chain_height = 0; // current height
    QString confirmationTime;
    int     numInputs = -1;
    int     numOutputs = -1;
    qint64 credited = -1;
    qint64 debited = -1;
    qint64 fee = -1;
    qint64 coinNano = 0; // Net diffrence, transaction weight
    QString kernel;
    QVector<QString> inputs;
    QVector<QString> outputs;
    QVector<WalletTransactionMessage> messages;

    static QString csvHeaders;

    WalletTransaction() = default;
    WalletTransaction(const WalletTransaction & other) = default;
    WalletTransaction & operator = (const WalletTransaction & other) = default;

    void setData(qint64 txIdx,
                      uint    transactionType,
                      QString txid,
                      QString address,
                      QString creationTime,
                      bool    confirmed,
                      qint64 ttlCutoffHeight,
                      qint64 output_height,
                      qint64 chain_height,
                      QString confirmationTime,
                      int     numInputs,
                      int     numOutputs,
                      qint64 credited,
                      qint64 debited,
                      qint64 fee,
                      qint64 coinNano,
                      QString kernel,
                      QVector<QString> inputs,
                      QVector<QString> outputs,
                      QVector<WalletTransactionMessage> messages);


    bool isValid() const {return txIdx>=0 && transactionType!=TRANSACTION_TYPE::NONE;}

    bool canBeCancelled() const { return (transactionType & TRANSACTION_TYPE::CANCELLED)==0 && !confirmed; }

    bool isCoinbase() const { return transactionType==TRANSACTION_TYPE::COIN_BASE; }

    // return transaction age (time interval from creation moment) in Seconds.
    qint64 calculateTransactionAge( const QDateTime & current ) const;

    // mark transaction as cancelled
    void cancelled() {
        transactionType |= TRANSACTION_TYPE::CANCELLED;
    }

    QString getTypeAsStr() const {
        QString res;

        if ( transactionType & TRANSACTION_TYPE::SEND )
            res += "Send";
        if ( transactionType & TRANSACTION_TYPE::RECEIVE )
            res += "Receive";
        if ( transactionType & TRANSACTION_TYPE::COIN_BASE )
            res += "CoinBase";

        if ( transactionType & TRANSACTION_TYPE::CANCELLED ) {
            if (!res.isEmpty())
                res += ", ";
            res += "Cancelled";
        }

        return res;
    }

    QString toStringShort() {
        using namespace util;

        return expandStrR( QString::number(txIdx), 3) +
                expandStrR(nano2one(coinNano), 8) +
                expandStrR( string2shortStrR(txid, 12), 12) +
                " " + creationTime;
    }

    static QString getCSVHeaders(const QStringList & extraHeaders) {
        return csvHeaders + (extraHeaders.isEmpty() ? "" : "," + extraHeaders.join(","));
    }

    // return transactions values formatted into a CSV string
    // Caller is responsible to make extraData to be CSV compatible
    QString toStringCSV(const QStringList & extraData) const;

    QJsonObject toJson() const;
    static WalletTransaction fromJson(const QJsonObject & doc);
};

    struct WalletUtxoSignature {
        qint64 coinNano = 0; // Output amount
        QString messageHash;
        QString pubKeyCompressed;
        QString messageSignature;

        void setData(qint64 _coinNano, // Output amount
                QString _messageHash,
                QString _pubKeyCompressed,
                QString _messageSignature);
    };

    struct SwapInfo {
        QString mwcAmount;
        QString secondaryAmount;
        QString secondaryCurrency;
        QString swapId;
        QString tag;
        qint64 startTime = 0;
        QString stateCmd; // state as command
        QString state; // State as string description
        QString action;
        qint64 expiration = 0;
        bool    isSeller;
        QString secondaryAddress;
        QString lastProcessError;

        void setData( QString mwcAmount, QString secondaryAmount, QString secondaryCurrency,
                      QString swapId, QString tag, qint64 startTime, QString stateCmd, QString state, QString action,
                      qint64 expiration, bool isSeller, QString secondaryAddress, QString lastProcessError );
    };

    struct SwapTradeInfo {
        QString swapId;
        QString tag;
        bool isSeller;
        double mwcAmount;
        double secondaryAmount;
        QString secondaryCurrency;
        QString secondaryAddress; // redeem/refund address

        double secondaryFee;
        QString secondaryFeeUnits;

        int mwcConfirmations;
        int secondaryConfirmations;
        int messageExchangeTimeLimit;
        int redeemTimeLimit;
        bool sellerLockingFirst;
        int mwcLockHeight;
        qint64 mwcLockTime;
        qint64 secondaryLockTime;

        QString communicationMethod;
        QString communicationAddress;

        QString electrumNodeUri; // Private electrumX URI


        void setData( QString swapId, QString tag, bool isSeller,  double mwcAmount, double secondaryAmount,
                    QString secondaryCurrency,  QString secondaryAddress, double secondaryFee,
                    QString secondaryFeeUnits, int mwcConfirmations, int secondaryConfirmations,
                    int messageExchangeTimeLimit, int redeemTimeLimit, bool sellerLockingFirst,
                    int mwcLockHeight, qint64 mwcLockTime, qint64 secondaryLockTime,
                    QString communicationMethod, QString communicationAddress, QString electrumNodeUri );
    };

    struct SwapExecutionPlanRecord {
        bool active = false;
        qint64 end_time = 0;
        QString name;

        void setData( bool active, qint64 end_time, QString name );
    };

    struct SwapJournalMessage {
        QString message;
        qint64 time = 0;

        void setData( QString message, qint64 time );
    };

    // Some startus booleans for 3 listeners
    struct ListenerStatus {
        bool mqs_started = false;
        bool mqs_healthy = false;
        bool tor_started = false;
        bool tor_healthy = false;

        ListenerStatus() = default;

        void setMqsStatus(bool _mqs_started, bool _mqs_healthy) {
            mqs_started = _mqs_started;
            mqs_healthy = _mqs_healthy;
        }

        void setTorStatus(bool _tor_started, bool _tor_healthy) {
            tor_started = _tor_started;
            tor_healthy = _tor_healthy;
        }

        ListenerStatus(const ListenerStatus & item) = default;
        ListenerStatus & operator = (const ListenerStatus & item) = default;

        bool isMqsHealthy() const {return mqs_started && mqs_healthy;}
        bool isTorHealthy() const {return tor_started && tor_healthy;}
    };

    struct IntegrityFees {
        bool confirmed = false;
        qint64 expiration_height = -1;
        qint64 ask_fee = -1;
        qint64 fee = -1;
        QString uuid;

        IntegrityFees() = default;
        IntegrityFees(const IntegrityFees & item) = default;
        IntegrityFees & operator = (const IntegrityFees & item) = default;

        IntegrityFees(bool _confirmed,
            qint64 _expiration_height,
            qint64 _ask_fee,
            qint64 _fee,
            QString _uuid) :
                confirmed(_confirmed),
                expiration_height(_expiration_height),
                ask_fee(_ask_fee),
                fee(_fee),
                uuid(_uuid)
        {}

        IntegrityFees(QString jsonString);
        IntegrityFees(QJsonObject json);

        double toDblFee() const {return fee / 1000000000.0;}

        QJsonObject toJSon() const;
        QString toJSonStr() const;
    };

    struct BroadcastingMessage {
        QString uuid;
        int broadcasting_interval = -1;
        qint64 fee = 0;
        QString message;
        int published_time;

        // {"broadcasting_interval":60,"fee":"10000000","message":"{}","published_time":49,"uuid":"7f0a6a89-5ad5-40cb-b204-0805ffcd1903"}
        BroadcastingMessage(const QJsonObject & json);
        BroadcastingMessage() = default;
        BroadcastingMessage(const BroadcastingMessage & item) = default;
        BroadcastingMessage & operator = (const BroadcastingMessage & item) = default;
    };

    struct MessagingStatus {
        QVector<BroadcastingMessage> broadcasting;
        bool connected = false;
        QStringList gossippub_peers;
        int received_messages = 0;
        QStringList topics;

        // {"broadcasting":[{"broadcasting_interval":60,"fee":"10000000","message":"{}","published_time":49,"uuid":"7f0a6a89-5ad5-40cb-b204-0805ffcd1903"}],"gossippub_peers":null,"received_messages":0,"topics":["swapmarketplace","testing"]}
        MessagingStatus(const QJsonObject & json);
        MessagingStatus() = default;
        MessagingStatus(const MessagingStatus & item) = default;
        MessagingStatus & operator = (const MessagingStatus & item) = default;

        // Status for logs
        QString toString() const;

        bool isConnected() const {return connected && gossippub_peers.size()>=3; }
    };

    struct ReceivedMessages {
        QString topic;   // Topic where we received the message
        qint64 fee;     // fee that was paid
        QString message; // message that received
        QString wallet;  // wallet onion address
        qint64 timestamp;

        ReceivedMessages(QString _topic,
                qint64 _fee,
                QString _message,
                QString _wallet,
                qint64 _timestamp) :
                topic(_topic),
                fee(_fee),
                message(_message),
                wallet(_wallet),
                timestamp(_timestamp) {}

        ReceivedMessages() = default;
        ReceivedMessages(const ReceivedMessages & item) = default;
        ReceivedMessages & operator = (const ReceivedMessages & item) = default;
    };

    struct SwapTradeDetails {
        SwapTradeInfo swap;
        QVector<SwapExecutionPlanRecord> executionPlan;
        QString currentAction;
        QVector<SwapJournalMessage> tradeJournal;
        QString error;
    };

    struct EthInfo {
        QString ethAddr;
        QString currency;
        QString balance;
    };

    struct AutoSwapStepResult {
        QString swapId;
        QString stateCmd;
        QString currentAction;
        QString currentState;
        QString lastProcessError;
        QVector<SwapExecutionPlanRecord> executionPlan;
        QVector<SwapJournalMessage> tradeJournal;
        QString error;
    };

    struct DecodedSlatepack {
        QJsonObject slate;
        QString content;
        QString sender;
        QString recipient;
        QString error;

        QJsonObject toJson();
        static DecodedSlatepack fromJson(QJsonObject obj);
    };


    struct ViewWalletOutputResult {
        QString commit;
        qint64 value = -1;
        qint64 height = -1;
        qint64 mmr_index = -1;
        bool is_coinbase = false;
        qint64 lock_height = -1;

        QJsonObject toJson() const;
    };

    struct ViewWallet {
        // Rewind Hash used to retrieve the outputs
        QString rewind_hash;
        // All outputs information that belongs to the rewind hash
        QVector<ViewWalletOutputResult> output_result;
        // total balance
        qint64 total_balance = -1;
        // last pmmr index
        qint64 last_pmmr_index = -1;

        QJsonObject toJson() const;
    };

    struct OwnershipProofValidation {
        /// Network name
        QString network;
        /// Message that was signed
        QString message;

        // Viewing key (optional)
        QString viewing_key;
        // Tor address (optional)
        QString tor_address;
        // MQS address (optional)
        QString mqs_address;
        // Error message for failed proof
        QString error;
    };

    struct NodeStatus {
        bool wasUpdated = false;
        bool internalNode = false;
        bool online = false;
        qint64 nodeHeight = 0;
        qint64 peerHeight = 0;
        qint64 totalDifficulty = 0;
        int connections = 0;

        bool isHealthy() const;
    };

    struct ResReceive {
        QString slatepack;
        QString tx_UUID;
    };

    struct MwcNodeConnectionDeprecated {
        void saveData(QDataStream & out) const;
        bool loadData(QDataStream & in);
    };

}

Q_DECLARE_METATYPE(wallet::Account);
Q_DECLARE_METATYPE(wallet::WalletTransaction);
Q_DECLARE_METATYPE(wallet::WalletOutput);
Q_DECLARE_METATYPE(wallet::SwapInfo);
Q_DECLARE_METATYPE(wallet::SwapTradeInfo);
Q_DECLARE_METATYPE(wallet::SwapExecutionPlanRecord);
Q_DECLARE_METATYPE(wallet::SwapJournalMessage);
Q_DECLARE_METATYPE(wallet::IntegrityFees);
Q_DECLARE_METATYPE(wallet::BroadcastingMessage);
Q_DECLARE_METATYPE(wallet::MessagingStatus);
Q_DECLARE_METATYPE(wallet::ReceivedMessages);
Q_DECLARE_METATYPE(wallet::ListenerStatus);
Q_DECLARE_METATYPE(wallet::SwapTradeDetails);
Q_DECLARE_METATYPE(wallet::EthInfo);
Q_DECLARE_METATYPE(wallet::AutoSwapStepResult);
Q_DECLARE_METATYPE(wallet::DecodedSlatepack);
Q_DECLARE_METATYPE(wallet::ViewWalletOutputResult);
Q_DECLARE_METATYPE(wallet::ViewWallet);
Q_DECLARE_METATYPE(wallet::OwnershipProofValidation);
Q_DECLARE_METATYPE(wallet::NodeStatus);
Q_DECLARE_METATYPE(wallet::ResReceive);

#endif //MWC_QT_WALLET_WALLET_OBJS_H

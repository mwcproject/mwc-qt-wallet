// Copyright 2020 The MWC Developers
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

//
// Created by Konstantin Bay on 5/30/20.
//

#ifndef MWC_QT_WALLET_WALLET_B_H
#define MWC_QT_WALLET_WALLET_B_H

#include <QObject>
#include "../core/Notification.h"
#include "../wallet/wallet.h"

namespace bridge {

class Wallet : public QObject {
    Q_OBJECT
public:
    explicit Wallet(QObject * parent = nullptr);
    ~Wallet();

    // return true if MQS is online
    Q_INVOKABLE bool getMqsListenerStatus();
    Q_INVOKABLE bool getKeybaseListenerStatus(); // Absolete
    // return true if Tor is online
    Q_INVOKABLE bool getTorListenerStatus();

    // return true if MQS is started
    Q_INVOKABLE bool isMqsListenerStarted();
    Q_INVOKABLE bool isKeybaseListenerStarted();  // Absolete
    // return true if Tor is started
    Q_INVOKABLE bool isTorListenerStarted();

    // Request start/stop listeners. Feedback should come with sgnUpdateListenerStatus
    Q_INVOKABLE void requestStartMqsListener();
    Q_INVOKABLE void requestStopMqsListener();
    Q_INVOKABLE void requestStartKeybaseListener();
    Q_INVOKABLE void requestStopKeybaseListener();
    Q_INVOKABLE void requestStartTorListener();
    Q_INVOKABLE void requestStopTorListener();

    // Return: signal  sgnRepost(int id, QString err)
    Q_INVOKABLE void repost(QString account, int id, bool fluff);

    // return values:
    // "true"  - listening
    // ""  - not listening, no errors
    // string  - not listening, error message
    Q_INVOKABLE QString getHttpListeningStatus();

    // Return a password hash for that wallet
    Q_INVOKABLE QString getPasswordHash();

    // return Total MWC amount as String. Formatted for GUI
    Q_INVOKABLE QString getTotalMwcAmount();

    // Get MQS address and index
    // Return: signal  sgnMwcAddressWithIndex
    Q_INVOKABLE void requestMqsAddress();
    // Change MWC box address to another from the chain. idx - index in the chain.
    // Return: signal  sgnMwcAddressWithIndex
    Q_INVOKABLE void requestChangeMqsAddress(int idx);
    // Generate next box address for the next index
    // Return: signal  sgnMwcAddressWithIndex
    Q_INVOKABLE void requestNextMqsAddress();
    // Get last known MQS address. It is good enough for cases when you don't expect address to be changed
    Q_INVOKABLE QString getMqsAddress();
    // Get last known Tor address. It is good enough for cases when you don't expect address to be changed
    Q_INVOKABLE QString getTorAddress();

    // Request a wallet address for file/http transactions
    // Return: signal  sgnFileProofAddress
    Q_INVOKABLE void requestFileProofAddress();

    // Request accounts info
    // includeAccountName - add Account names
    // includeSpendableInfo - add String about Spendables
    // includeAccountFullInfo - add account full info
    Q_INVOKABLE QVector<QString> getWalletBalance(bool includeAccountName, bool includeSpendableInfo,  bool includeAccountFullInfo);
    // Get current account name for the wallet
    Q_INVOKABLE QString getCurrentAccountName();
    // Change current account
    Q_INVOKABLE void switchAccount( QString accountName );

    // Initiate wallet balance update. Please note, update happens in the backgorund and on events.
    // When done onWalletBalanceUpdated will be called.
    Q_INVOKABLE void requestWalletBalanceUpdate();

    // Request list of outputs for the account.
    // Respond will be with sgnOutputs
    Q_INVOKABLE void requestOutputs(QString account, bool show_spent, bool enforceSync);

    // Show all transactions for current account
    // Respond: sgnTransactions( QString account, QString height, QVector<QString> Transactions);
    Q_INVOKABLE void requestTransactions(QString account, bool enforceSync);

    // get Extended info for specific transaction
    // Respond:  sgnTransactionById( bool success, QString account, QString height, QString transaction,
    //                            QVector<QString> outputs, QVector<QString> messages );
    Q_INVOKABLE void requestTransactionById(QString account, QString txIdx );

    // Cancel transaction by id
    // Respond: sgnCancelTransacton( bool success, QString trIdx, QString errMessage )
    Q_INVOKABLE void requestCancelTransacton(QString account, QString txIdx);

    // Set acount to receive the coins
    Q_INVOKABLE void setReceiveAccount(QString account);
    // Get current account that receive coins
    Q_INVOKABLE QString getReceiveAccount();


    // Generating transaction proof for transaction.
    // Respond: sgnExportProofResult( bool success, QString fn, QString msg );
    Q_INVOKABLE void generateTransactionProof( QString transactionId, QString resultingFileName );

    // Verify the proof for transaction
    // Respond: sgnVerifyProofResult( bool success, QString msg );
    Q_INVOKABLE void verifyTransactionProof( QString proofFileName );

    // Request Node status
    // Return true if task was scheduled (wallet is unlocked)
    // Respond: sgnNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections )
    Q_INVOKABLE bool requestNodeStatus();

    // Create another account, note no delete exist for accounts
    // Check Signal:  sgnAccountCreated
    Q_INVOKABLE void createAccount( QString accountName );

    // Rename account
    // Check Signal: sgnAccountRenamed(bool success, QString errorMessage);
    Q_INVOKABLE void renameAccount(QString oldName, QString newName);

signals:
    // Updates from the wallet and notification system
    void sgnNewNotificationMessage(int level, QString message); // level: notify::MESSAGE_LEVEL values
    void sgnConfigUpdate();
    // keybaseOnline  is absolete, always false
    void sgnUpdateListenerStatus(bool mwcOnline, bool keybaseOnline, bool tor);
    void sgnHttpListeningStatus(bool listening, QString additionalInfo);
    void sgnUpdateNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, double totalDifficulty, int connections );
    void sgnUpdateSyncProgress(double progressPercent);
    void sgnWalletBalanceUpdated();
    void sgnLoginResult(bool ok);
    void sgnLogout();

    // Some of listeners was started, stopped
    // kbTry  is absolete, allways false value
    void sgnListenerStartStop(bool mqTry, bool _kbTry, bool tor);

    // Get MWC MQ address with index
    void sgnMwcAddressWithIndex(QString mwcAddress, int idx);

    // Get Tor address. Address will be assigned when Tor listener is started.
    // Empty address will be assigned when Tor listener is stopped.
    void sgnTorAddress(QString tor);

    // Get wallet provable address. Please note, address will be changed
    // When address index will be changed. Normally it is the same as a tor address
    void sgnFileProofAddress(QString proofAddress);

    // Outputs requested form the wallet.
    // outputs are in Json format, see wallet::WalletOutput for details
    void sgnOutputs( QString account, bool showSpent, QString height, QVector<QString> outputs);

    //  Transactions from the requestTransactions request
    // Transactions are in Json format, see wallet::WalletTransaction for details
    void sgnTransactions( QString account, QString height, QVector<QString> transactions);
    // Transaction from getTransactionById request
    // transaction: JSON for wallet::WalletTransaction
    // outputs: JSON for  wallet::WalletOutput
    void sgnTransactionById( bool success, QString account, QString height, QString transaction,
                            QVector<QString> outputs, QVector<QString> messages );
    // Respond from cancelTransacton
    void sgnCancelTransacton( bool success, QString account, QString trIdx, QString errMessage );

    // respond from generateTransactionProof
    void sgnExportProofResult( bool success, QString fn, QString msg );

    // respond from verifyTransactionProof
    void sgnVerifyProofResult( bool success, QString fn, QString msg );

    // Node status respond,  requestNodeStatus()
    void sgnNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, QString totalDifficulty, int connections );

    // New account is created, createAccount
    void sgnAccountCreated( QString newAccountName);
    // Account is renamed, renameAccount
    void sgnAccountRenamed(bool success, QString errorMessage);

    // respobd from repost.  OK is err is empty
    void sgnRepost(int txIdx, QString err);
private slots:
    // Signals that comes from wallet & notification system
    void onNewNotificationMessage(notify::MESSAGE_LEVEL level, QString message);
    void onConfigUpdate();
    void onListeningStartResults( bool mqTry, bool tor,
                                  QStringList errorMessages, bool initialStart ); // error messages, if get some
    void onListeningStopResult(bool mqTry, bool tor,
                               QStringList errorMessages );
    void onUpdateListenerStatus(bool mqsOnline, bool torOnline);
    void onHttpListeningStatus(bool listening, QString additionalInfo);
    void onUpdateNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );
    void onUpdateSyncProgress(double progressPercent);
    void onWalletBalanceUpdated();
    void onLoginResult(bool ok);
    void onLogout();
    void onMwcAddressWithIndex(QString mwcAddress, int idx);
    void onTorAddress(QString tor);
    void onFileProofAddress(QString address);
    void onOutputs( QString account, bool showSpent, int64_t height, QVector<wallet::WalletOutput> outputs);

    void onTransactions( QString account, int64_t height, QVector<wallet::WalletTransaction> Transactions);
    void onCancelTransacton( bool success, QString account, int64_t trIdx, QString errMessage );
    void onTransactionById( bool success, QString account, int64_t height, wallet::WalletTransaction transaction,
                                QVector<wallet::WalletOutput> outputs, QVector<QString> messages );

    void onExportProof( bool success, QString fn, QString msg );
    void onVerifyProof( bool success, QString fn, QString msg );
    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

    void onAccountCreated( QString newAccountName);
    void onAccountRenamed(bool success, QString errorMessage);

    void onRepost(int txIdx, QString err);
};

}

#endif //MWC_QT_WALLET_WALLET_B_H

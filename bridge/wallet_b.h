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
#include <QJsonArray>

namespace bridge {

class Wallet : public QObject {
    Q_OBJECT
public:
    explicit Wallet(QObject * parent = nullptr);
    ~Wallet();

    // Return true if wallet has this password. Wallet might not have password (has empty password) if it was created manually.
    // Expected that the wallet is already open,
    Q_INVOKABLE bool checkPassword(QString password);

    // Return 4 values:
    // [0]  mqs_started
    // [1]  mqs_healthy
    // [2]  tor_started
    // [3]  tor_healthy
    Q_INVOKABLE QVector<bool> getListenerStatus();

    // Request MQS address
    Q_INVOKABLE QString getMqsAddress();

    // Request Tor address
    Q_INVOKABLE QString getTorSlatepackAddress();

    // request current address index
    Q_INVOKABLE int getAddressIndex();

    // Note, set address index does update the MQS and Tor addresses
    // The Listeners, if running, will be restarted automatically
    Q_INVOKABLE void setAddressIndex(int index);

    // Request accounts info
    // includeAccountName - return Account names
    // includeAccountFullInfo - return account full info
    Q_INVOKABLE QVector<QString> getWalletBalance(bool includeAccountName, bool includeAccountPath, bool includeSpendableInfo, bool includeAccountFullInfo);

    // return current account path
    Q_INVOKABLE QString getCurrentAccountId();

    // Switch to different account
    Q_INVOKABLE void switchAccountById(QString accountPath);

    // Show outputs for the wallet
    Q_INVOKABLE QJsonArray getOutputs(QString accountPath, bool show_spent);

    // Show outputs for the wallet
    Q_INVOKABLE QJsonArray getOutputsByCommits(QString accountPath, QVector<QString> commits);

    // Set account that will receive the funds
    Q_INVOKABLE void setReceiveAccountById(QString accountPath);
    Q_INVOKABLE QString getReceiveAccountPath();

    // Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
    Q_INVOKABLE QJsonObject decodeSlatepack(QString slatepackContent);

    // Show all transactions for current account
    Q_INVOKABLE QJsonArray getTransactions( QString accountPath );

    // Request single transaction by UUID, any account
    Q_INVOKABLE QJsonObject getTransactionByUUID( QString tx_uuid );

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    // Return error or JsonObject
    Q_INVOKABLE QString generateTransactionProof( QString transactionUuid );

    // Verify the proof for transaction
    // Return error or JsonObject
    Q_INVOKABLE QString verifyTransactionProof( QString proof );

    // Check if slatepack data does exist
    Q_INVOKABLE bool hasSendSlatepack(QString txUUID);
    Q_INVOKABLE bool hasReceiveSlatepack(QString txUUID);
    Q_INVOKABLE bool hasFinalizedData(QString txUUID);

    // Request to show the slatepack data
    Q_INVOKABLE void viewSendSlatepack(QString txUUID);
    Q_INVOKABLE void viewReceiveSlatepack(QString txUUID);

    // Cancel TX by UUID (in case of multi accouns, we want to cancel both)
    // Return error string
    Q_INVOKABLE QString cancelTransacton(QString txUUID);

    // Repost the transaction.
    // Return Error
    Q_INVOKABLE QString repostTransaction(QString txUUID, bool fluff);

    // Rename account
    Q_INVOKABLE void renameAccountById( QString accountPath, QString newAccountName);

    // Create another account, note no delete exist for accounts
    // Return account path
    Q_INVOKABLE QString createAccount( const QString & accountName );

    // Get rewind hash
    Q_INVOKABLE QString viewRewindHash();

    // Return Total MWC and Unconfirmed MWC
    // [0] - total MWC whole
    // [1] - total MWC full fractions (9 digits)
    // [2] - unconfirmed
    Q_INVOKABLE QVector<QString> getTotalAmount();

    Q_INVOKABLE void requestFaucetMWC();
signals:
    void sgnConfigUpdate();
    void sgnLogin();
    void sgnLogout();
    void sgnScanProgress( QString responseId, QJsonObject statusMessage );
    void sgnScanDone( QString responseId, bool fullScan, int height, QString errorMessage );
    void sgnSend( bool success, QString error, QString tx_uuid, QString tag );
    void sgnSlateReceivedFrom(QString slate, QString mwc, QString fromAddr, QString message );
    void sgnScanRewindHash( QString responseId, QJsonObject walletOutputs, QString errors );
    void sgnWalletBalanceUpdated();
    void sgnFaucetMWCDone(bool success);

private slots:
    void onConfigUpdate();
    void onLogin();
    void onLogout();
    void onScanProgress( QString responseId, QJsonObject statusMessage );
    void onScanDone( QString responseId, bool fullScan, int height, QString errorMessage );
    void onSend( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag );
    void onSlateReceivedFrom(QString slate, qint64 mwc, QString fromAddr, QString message );
    void onScanRewindHash( QString responseId, wallet::ViewWallet walletOutputs, QString errors );
    void onWalletBalanceUpdated();
    void onFaucetMWCDone(bool success);

private:
};

}

#endif //MWC_QT_WALLET_WALLET_B_H

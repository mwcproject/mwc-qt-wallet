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

#include "MockWallet.h"
#include "../util/crypto.h"

namespace wallet {

const QString MWC_MQS_ADDRESS = "xminFUjbKtaynDuX9KzSz74RAd5Wj5KeeiQ5cA2i4Z5DFYR3PZFN";
const QString MWC_MQS_ADDRESS2 = "q5ZV7vjqSxTyFjN1TDgShecfDiGaaCsV14v32j7VgiAimj3jsWVK";
const QVector<QString> TEST_SEED = QVector<QString>{
        "angry","animal","ankle","announce",
        "actress", "actual", "adapt", "add",
        "addict", "address", "adjust", "admit",
        "adult", "advance", "advice", "aerobic",
        "affair", "afford", "afraid", "again",
        "age", "agent", "agree", "ahead"};

MockWallet::MockWallet(core::AppContext *_appContext) {
    appContext = _appContext;
    mwcAddress = MWC_MQS_ADDRESS;

    AccountInfo acc1;
    acc1.setData("default",
            5000000000,
            1000000000,
            1000000000,
            3000000000,
            12345,
            false);
    accountInfo.push_back(acc1);

    AccountInfo acc2;
    acc1.setData("Bob",
                 8000000000,
                 2000000000,
                 2000000000,
                 4000000000,
                 12345,
                 false);
    accountInfo.push_back(acc1);

}

MockWallet::~MockWallet() {}

// Create new wallet and generate a seed for it
// Check signal: onNewSeed( seed [] )
void MockWallet::start2init(QString pass) {
    passwordHash = crypto::calcHSA256Hash(pass);
    emit onNewSeed( TEST_SEED);
}

// Recover the wallet with a mnemonic phrase
// recover wallet with a passphrase:
// Check Signals: onRecoverProgress( int progress, int maxVal );
// Check Signals: onRecoverResult(bool ok, QString newAddress );
void MockWallet::start2recover(const QVector<QString> &seed, QString pass) {
    Q_UNUSED(seed)
    passwordHash = crypto::calcHSA256Hash(pass);

    emit onRecoverProgress( 2, 10 );
    emit onRecoverProgress( 4, 10 );
    emit onRecoverProgress( 7, 10 );
    emit onRecoverProgress( 10, 10 );

    emit onRecoverResult(true, true, MWC_MQS_ADDRESS, {});
}

// Check signal: onLoginResult(bool ok)
void MockWallet::loginWithPassword(QString pass) {
    passwordHash = crypto::calcHSA256Hash(pass);
    emit onLoginResult(true);
}

// Current seed for runnign wallet
// Check Signals: onGetSeed(QVector<QString> seed);
void MockWallet::getSeed(const QString & walletPassword) {
    Q_UNUSED(walletPassword)
    emit onGetSeed(TEST_SEED);
}

// Start listening through services
// Check Signal: onStartListening
void MockWallet::listeningStart(bool startMq, bool startKb, bool startTor, bool initialStart) {
    if (startMq) {
        listener_mwcmqs = true;
        emit onListeningStartResults(true, false, false, {}, initialStart);
    }
    if (startKb) {
        listener_keybase = true;
        emit onListeningStartResults(false, true, false, {}, initialStart);
    }
    if (startTor) {
        listener_tor = true;
        emit onListeningStartResults(false, false, true, {}, initialStart);
    }
}

// Stop listening through services
// Check signal: onListeningStopResult
void MockWallet::listeningStop(bool stopMq, bool stopKb, bool stopTor) {
    if (stopMq) {
        listener_mwcmqs = false;
        emit onListeningStopResult(true, false, false, {});
    }
    if (stopKb) {
        listener_keybase = false;
        emit onListeningStopResult(false, true, false, {});
    }
    if (stopTor) {
        listener_tor = stopTor;
        emit onListeningStopResult(false, false, true, {});
    }
}

// Get latest Mwc MQ address that we see
QString MockWallet::getMqsAddress()
{
    return mwcAddress;
}
QString MockWallet::getTorAddress()
{
    return "http://87658976345873482345.onion";
}


// Get MWC box <address, index in the chain>
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MockWallet::getMwcBoxAddress() {
    emit onMwcAddressWithIndex(mwcAddress, 1);
}

// Change MWC box address to another from the chain. idx - index in the chain.
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MockWallet::changeMwcBoxAddress(int idx) {
    mwcAddress = MWC_MQS_ADDRESS2;
    onMwcAddressWithIndex(mwcAddress, idx);
}

// Generate next box address
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MockWallet::nextBoxAddress() {
    if (mwcAddress == MWC_MQS_ADDRESS) {
        mwcAddress = MWC_MQS_ADDRESS;
        emit onMwcAddressWithIndex(mwcAddress, 3);
    }
    else {
        mwcAddress = MWC_MQS_ADDRESS2;
        emit onMwcAddressWithIndex(mwcAddress, 4);
    }
}

// Get all accounts with balances. Expected that Wallet allways maintain them in a cache.
// This info needed in many cases and we don't want spend time every time for that.
QVector<AccountInfo> MockWallet::getWalletBalance(bool filterDeleted) const {
    Q_UNUSED(filterDeleted)
    return accountInfo;
}

static QMap<QString, QVector<wallet::WalletOutput> > emptyOutputs;

// Get outputs that was collected for this wallet. Outputs should be ready with balances
const QMap<QString, QVector<wallet::WalletOutput> > &  MockWallet::getwalletOutputs() const {
    return emptyOutputs;
}

// Request Wallet balance update. It is a multistep operation
// Check signal: onWalletBalanceUpdated
//          onWalletBalanceProgress
void MockWallet::updateWalletBalance(bool enforceSync, bool showSyncProgress, bool skipSync) {
    Q_UNUSED(enforceSync)
    Q_UNUSED(showSyncProgress)
    Q_UNUSED(skipSync)

    emit onWalletBalanceProgress( 2, 4 );
    emit onWalletBalanceProgress( 4, 4 );

    emit onWalletBalanceUpdated();
}


// Create another account, note no delete exist for accounts
// Check Signal:  onAccountCreated
void MockWallet::createAccount(const QString &accountName) {
    emit onAccountCreated(accountName);
}

// Switch to different account
void MockWallet::switchAccount(const QString &accountName) {
    Q_UNUSED(accountName)
}

// Rename account
// Check Signal: onAccountRenamed(bool success, QString errorMessage);
void MockWallet::renameAccount(const QString &oldName, const QString &newName) {
    Q_UNUSED(oldName)
    Q_UNUSED(newName)

    emit onAccountRenamed(true, "");
}

// Check and repair the wallet. Will take a while
// Check Signals: onRecoverProgress( int progress, int maxVal );
// Check Signals: onCheckResult(bool ok, QString errors );
void MockWallet::check(bool wait4listeners) {
    Q_UNUSED(wait4listeners)

    emit onRecoverProgress( 1, 4 );
    emit onRecoverProgress( 3, 4 );

    emit onCheckResult(true, "");
}

// Get current configuration of the wallet. will read from wallet713.toml file
const WalletConfig & MockWallet::getWalletConfig() {
    static WalletConfig config;

    config.setData("Floonet",
            "mock_wallet_data",
            "mwcmqsDomain",
            "keyBasePath",
            false,
            "127.0.0.1",
            "",
            "",
            "");
    return config;
}

// Get configuration form the resource file.
const WalletConfig & MockWallet::getDefaultConfig() {
    return getWalletConfig();
}

// Update wallet config. Will update config and restart the mwc713.
// Note!!! Caller is fully responsible for input validation. Normally mwc713 will start, but some problems might exist
//          and caller suppose listen for them
// If return true, expected that wallet will need to have password input.
// Check signal: onConfigUpdate()
bool MockWallet::setWalletConfig(const WalletConfig &config, core::AppContext *appContext, node::MwcNode *mwcNode) {
    Q_UNUSED(config)
    Q_UNUSED(appContext)
    Q_UNUSED(mwcNode)

    emit onConfigUpdate();
    return false;
}

// Status of the node
// return true if task was scheduled
// Check Signal: onNodeStatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections )
bool MockWallet::getNodeStatus() {
    emit onNodeStatus( true, "", 12345, 12345, 1234567, 5 );
    return true;
}

// Set account that will receive the funds
// Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
void MockWallet::setReceiveAccount(QString account) {
    Q_UNUSED(account)
    emit onSetReceiveAccount( true, account );
}

// Cancel transaction
// Check Signal:  onCancelTransacton
void MockWallet::cancelTransacton(QString account, int64_t txIdx) {
    Q_UNUSED(txIdx)

    emit onCancelTransacton2( true, account, txIdx, "" );
}

// Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
// Check Signal: onExportProof( bool success, QString fn, QString msg );
void MockWallet::generateMwcBoxTransactionProof(int64_t transactionId, QString resultingFileName)
{
    Q_UNUSED(transactionId)
    Q_UNUSED(resultingFileName)

    emit onExportProof( true, resultingFileName, "" );
}

// Verify the proof for transaction
// Check Signal: onVerifyProof( bool success, QString msg );
void MockWallet::verifyMwcBoxTransactionProof(QString proofFileName) {
    Q_UNUSED(proofFileName)

    emit onVerifyProof( true, proofFileName, "" );
}

// Init send transaction with file output
// Check signal:  onSendFile
void MockWallet::sendFile(const QString &account, int64_t coinNano, QString message, QString fileTx,
                          int inputConfirmationNumber, int changeOutputs,
                          const QStringList &outputs) {
    Q_UNUSED(account)
    Q_UNUSED(coinNano)
    Q_UNUSED(message)
    Q_UNUSED(fileTx)
    Q_UNUSED(inputConfirmationNumber)
    Q_UNUSED(changeOutputs)
    Q_UNUSED(outputs)

    emit onSendFile( true, {}, fileTx );
}

// Receive transaction. Will generate *.response file in the same dir
// Check signal:  onReceiveFile
void MockWallet::receiveFile(QString fileTx, QString identifier) {
    Q_UNUSED(identifier)

    emit onReceiveFile( true, {}, fileTx, fileTx+".respond" );
}

// finalize transaction and broadcast it
// Check signal:  onFinalizeFile
void MockWallet::finalizeFile(QString fileTxResponse, bool fluff) {
    Q_UNUSED(fluff)
    emit onFinalizeFile( true, {}, fileTxResponse );
}

// submit finalized transaction. Make sense for cold storage => online node operation
// Check Signal: onSubmitFile(bool ok, String message)
void MockWallet::submitFile(QString fileTx) {
    emit onSubmitFile(true, "", fileTx);
}

// Airdrop special. Generating the next Pablic key for transaction
// wallet713> getnextkey --amount 1000000
// "Identifier(0300000000000000000000000600000000), PublicKey(38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79)"
// Check Signal: onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor);
void MockWallet::getNextKey(int64_t amountNano, QString btcaddress, QString airDropAccPassword) {
    Q_UNUSED(amountNano)
    Q_UNUSED(btcaddress)
    Q_UNUSED(airDropAccPassword)

    emit onGetNextKeyResult( true, "0300000000000000000000000600000000",
            "38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79",
            "", btcaddress, airDropAccPassword);
}

// Send some coins to address.
// Before send, wallet always do the switch to account to make it active
// Check signal:  onSend
// coinNano == -1  - mean All
void MockWallet::sendTo(const QString &account, int64_t coinNano, const QString &address,
                        const QString &apiSecret,
                        QString message, int inputConfirmationNumber, int changeOutputs,
                        const QStringList &outputs, bool fluff) {
    Q_UNUSED(account)
    Q_UNUSED(coinNano)
    Q_UNUSED(address)
    Q_UNUSED(apiSecret)
    Q_UNUSED(message)
    Q_UNUSED(inputConfirmationNumber)
    Q_UNUSED(changeOutputs)
    Q_UNUSED(outputs)
    Q_UNUSED(fluff)

    emit onSend( true, {}, address, 4, "0000-1111-2222-3333", util::nano2one(coinNano) );
}

// Show outputs for the wallet
// Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> outputs)
void MockWallet::getOutputs(QString account, bool show_spent, bool enforceSync) {
    Q_UNUSED(show_spent)
    Q_UNUSED(enforceSync)

    QVector<WalletOutput> outputs;
    outputs.push_back(WalletOutput::create("01234327643847563487654386",
            "123",
            "1234",
            "1234",
            "Sent",
            false,
            "4",
            1000000000,
            2));
    emit onOutputs2( account, show_spent, 12345, outputs);
}

// Show all transactions for current account
// Check Signal: onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions)
void MockWallet::getTransactions(QString account, bool enforceSync) {
    Q_UNUSED(enforceSync)

    WalletTransaction tx;
    tx.setData(2,
            WalletTransaction::TRANSACTION_TYPE::SEND,
            "4",
            "address",
            "02-02-2020 10:00",
            true,
            1234,
            1234,
            "02-02-2020 10:00",
            1,
            2,
            1000000000,
            1000000000,
            10000000,
            1000000000,
            false,
            "3746538765238745643");

    emit onTransactions( account, 12345, {tx});
}

// get Extended info for specific transaction
// Check Signal: onTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages )
void MockWallet::getTransactionById(QString account, int64_t txIdx) {
    Q_UNUSED(txIdx)

    WalletTransaction tx;
    tx.setData(2,
               WalletTransaction::TRANSACTION_TYPE::SEND,
               "4",
               "address",
               "02-02-2020 10:00",
               true,
               1234,
               1234,
               "02-02-2020 10:00",
               1,
               2,
               1000000000,
               1000000000,
               10000000,
               1000000000,
               false,
               "3746538765238745643");

    emit onTransactionById( true, account, 1234, tx, {}, {} );
}

// Read all transactions for all accounts. Might take time...
// Check Signal: onAllTransactions( QVector<WalletTransaction> Transactions)
void MockWallet::getAllTransactions() {
    WalletTransaction tx;
    tx.setData(2,
               WalletTransaction::TRANSACTION_TYPE::SEND,
               "4",
               "address",
               "02-02-2020 10:00",
               true,
               1234,
               1234,
               "02-02-2020 10:00",
               1,
               2,
               1000000000,
               1000000000,
               10000000,
               1000000000,
               false,
               "3746538765238745643");

    emit onAllTransactions( {tx} );
}

// Get root public key with signed message. Message is optional, can be empty
// Check Signal: onRootPublicKey( QString rootPubKey, QString message, QString signature )
void MockWallet::getRootPublicKey(QString message2sign) {
    if (message2sign.isEmpty()) {
        emit onRootPublicKey(true, "",
                             "38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79",
                             "",
                             "");
    }
    else {
        emit onRootPublicKey(true, "",
                             "38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79",
                             message2sign,
                             "38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79");
    }
}

}

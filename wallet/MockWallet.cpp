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
#include <QMap>

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
void MockWallet::start2init(QString pass, int seedLength) {
    passwordHash = crypto::calcHSA256Hash(pass);
    Q_ASSERT(TEST_SEED.length() == seedLength);
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
void MockWallet::listeningStart(bool startMq, bool startTor, bool initialStart) {
    if (startMq) {
        listener_mwcmqs = true;
        emit onListeningStartResults(true, false, {}, initialStart);
    }
    if (startTor) {
        listener_tor = true;
        emit onListeningStartResults(false, true, {}, initialStart);
    }
}

// Stop listening through services
// Check signal: onListeningStopResult
void MockWallet::listeningStop(bool stopMq, bool stopTor) {
    if (stopMq) {
        listener_mwcmqs = false;
        emit onListeningStopResult(true, false, {});
    }
    if (stopTor) {
        listener_tor = stopTor;
        emit onListeningStopResult(false, true, {});
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
            false,
            "127.0.0.1",
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
bool MockWallet::setWalletConfig(const WalletConfig &config, bool canStartNode) {
    Q_UNUSED(config)
    Q_UNUSED(canStartNode)

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
    receiveAccount = account;
    emit onSetReceiveAccount( true, account );
}

QString MockWallet::getReceiveAccount() {
    return receiveAccount;
}

// Cancel transaction
// Check Signal:  onCancelTransacton
void MockWallet::cancelTransacton(QString account, int64_t txIdx) {
    Q_UNUSED(txIdx)

    emit onCancelTransacton( true, account, txIdx, "" );
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
                          const QStringList &outputs, int ttl_blocks, bool generateProof) {
    Q_UNUSED(account)
    Q_UNUSED(coinNano)
    Q_UNUSED(message)
    Q_UNUSED(fileTx)
    Q_UNUSED(inputConfirmationNumber)
    Q_UNUSED(changeOutputs)
    Q_UNUSED(outputs)
    Q_UNUSED(ttl_blocks)
    Q_UNUSED(generateProof)

    emit onSendFile( true, {}, fileTx );
}

// Receive transaction. Will generate *.response file in the same dir
// Check signal:  onReceiveFile
void MockWallet::receiveFile(QString fileTx, QString description, QString identifier) {
    Q_UNUSED(identifier)
    Q_UNUSED(description)

    emit onReceiveFile( true, {}, fileTx, fileTx+".respond" );
}

// finalize transaction and broadcast it
// Check signal:  onFinalizeFile
void MockWallet::finalizeFile(QString fileTxResponse, bool fluff) {
    Q_UNUSED(fluff)
    emit onFinalizeFile( true, {}, fileTxResponse );
}

// Init send transaction with file output
// Check signal:  onSendSlatepack
void MockWallet::sendSlatepack( const QString &account, int64_t coinNano, QString message,
                            int inputConfirmationNumber, int changeOutputs, const QStringList & outputs,
                            int ttl_blocks, bool generateProof,
                            QString slatepackRecipientAddress, // optional. Encrypt SP if it is defined.
                            bool isLockLater,
                            QString tag ) {
    Q_UNUSED(account)
    Q_UNUSED(coinNano)
    Q_UNUSED(message)
    Q_UNUSED(inputConfirmationNumber)
    Q_UNUSED(changeOutputs)
    Q_UNUSED(outputs)
    Q_UNUSED(ttl_blocks)
    Q_UNUSED(generateProof)
    Q_UNUSED(slatepackRecipientAddress)
    Q_UNUSED(isLockLater)
    emit onSendSlatepack( tag, "", "BEGINSLATE_BIN. PFhuXojFiTEBiLF NnpPMX7ocEBp7Ch NrKHGuW6aiQW6cn 77PSks4oLRfZLmu Qb2GhowApRjWDJi WGdPDF2EkikDpmh Z6GX3E4haRUqUwz hXQqHrMqeQNE6Mm 55B2eL9227VFgAd c5Gr5Gs. ENDSLATE_BIN." );
}

// Receive transaction. Will generate *.response file in the same dir
// Check signal:  onReceiveSlatepack
void MockWallet::receiveSlatepack( QString slatePack, QString description, QString tag) {
    Q_UNUSED(slatePack)
    Q_UNUSED(description)
    emit onReceiveSlatepack( tag, "", "BEGINSLATE_BIN. 2RaFmV3oDFPThCu t3kFBTSSYRXwKY9 AhAXnbfiJmbxSbt d51GKkB5yN1VoMG oSWWQG8gsuahsj8 QrjTxd6TbTps5ZH vjHjpffBovxYgpP ZFMeUUWEzvfbLc2 x1EPdzLVBEvALih AsvEc5yoh3tcS91 nirzrkAzK3KkrXv PQkw71VndsNQbQR YGMipBUKyyLNxxp mX5wKayQAVMZbbw H8jR16dy3htGktg Hq3eKiJLqqqi8FM iyxTCQgj5y6dPju fUe6mw58zxjHD8P XRVG6faKDLtcBHg DpTVBZe3M4xSoWn HvFdHj1jzG8xAAd 2K4cxe854qN77aQ JzJCSRmLqfvmNd6 gkRUYh5h8WoyL5k ZyW6mgGMhqFQVuc QM6XKCKfvMJkdWw skqt68WpNzLp8UJ G7fiHD3Zc3wYFwP 6DRBV2cP5y47kRb 4pbTp4BuqgvdpTK 6sqb1YhYreWAe7j asc5gbB5mRvQhp5 GdDeSGjP6SoaaUp 8iHkQYbjPUfLMZL vW4S7a9Ffumw65K rJpNkdKe2wGrsAb 68KUc4WCPg37kyh 1Fc4t8KMSH1ntMp DbbYFKtNteX2Jmf 6mQdAziLP7Ck2uS NrhpGs6uQCe8mMy FTCfiYHS9HpZVaV 2i4Zykjtn16fGuy QM6EHEZhFNU1Gp8 GGNYWjruYhHF1Un JoSY6ZRWWeBV2Tm yGijqhm8X7LdWUK HbPP3h8NQULW1uh znkY32wYAwrq8dY VPp5TH8kn9pKGTs 4DcayGws8YspfLJ ombowt6QLgwsePY t7sXMpxbYunm1xb tAsAoC9Bne5p9Yd psqqHoBH8PZLaD4 WF3VodiKyYKGfoQ vNHmySFGu3ekXt3 V4PVbKNX1g4jJ5w hyoJrL9u8yuQnNL pLsBDALEv7REGqM qd8GFyryHTY1cAs jioFb8MpP3gGotK 1KJ3cqmn6b6NGaB m6QFFZMNei3WqKL PTv2fTWTQ22UUWF TEVzuEmbzhM9HCd R83MAk9VReusdvP RGDwYixKNgRFeNa ekgyzTk9XLCgQdo d8y9dwH9Cui5RdP RXLrVQqUM4euYd1 32fJSxZgpsj7L6Z yStjWEszSC1nZSx dHuZb5YEtr4TdUd GDUy7Hn8F8Ribp3 DeGCRoLYk3ac3Ax mxABwH1i5YpoFwr 9tHNTAMyXysmxgi xMMhjAUxPudx3UC xWJ4LL19iKCz7mF JwtgNq6DNuxH6a2 soowwhcUBi5KqiK 8PfroU79rNbykLv 2164E4yBXBtPTtC z4rXy5cA2X8zxRd uZxgHFzayaQ9UoV 2KP1YUja1LWYHp3 PwktUfFhFJaQHZp FvmyuTpZjgmwJ2S sTYcpufBrpHDts8 DYQX9eLRpScD2Kg FfbBSRqepinibAC gGAExNRLUmpc583 bc9WSm2ji9rCU1d 7wDxgyFbhqQLmhz n2PW1hMmYKPiHJf f6BsZPmHxWkyRLh M4Dn7BFfntLJUSy Ri7. ENDSLATE_BIN." );
}

// finalize transaction and broadcast it
// Check signal:  onFinalizeSlatepack
void MockWallet::finalizeSlatepack( QString slatepack, bool fluff, QString tag ) {
    Q_UNUSED(slatepack)
    Q_UNUSED(fluff)
    emit onFinalizeSlatepack( tag, "", "12345-12345-12345-12345" );
}

// Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
// Check Signal: onDecodeSlatepack( QString tag, QString error, QString slateJSon, QString content, QString sender, QString receiver )
void MockWallet::decodeSlatepack(QString slatepackContent, QString tag) {
    Q_UNUSED(slatepackContent)
    emit onDecodeSlatepack( tag, "", slatepackContent, "{}", "SendInitial", "xxxxSenderxxxxx", "xxxxxRecipientxxxxxx" );
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
                        const QStringList &outputs, bool fluff, int ttl_blocks, bool generateProof, QString expectedproofAddress) {
    Q_UNUSED(account)
    Q_UNUSED(coinNano)
    Q_UNUSED(address)
    Q_UNUSED(apiSecret)
    Q_UNUSED(message)
    Q_UNUSED(inputConfirmationNumber)
    Q_UNUSED(changeOutputs)
    Q_UNUSED(outputs)
    Q_UNUSED(fluff)
    Q_UNUSED(ttl_blocks)
    Q_UNUSED(generateProof)
    Q_UNUSED(expectedproofAddress)

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
    emit onOutputs( account, show_spent, 12345, outputs);
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
void MockWallet::getTransactionById(QString account, QString txIdxOrUUID) {
    Q_UNUSED(txIdxOrUUID)

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

// Repost the transaction. Optionally fluff.
// index is the tx_index in the tx_log.
void MockWallet::repost(QString account, int index, bool fluff) {
    Q_UNUSED(account)
    Q_UNUSED(index)
    Q_UNUSED(fluff)
    emit onRepost(index, "");
}

// Request proof address for files
// Check signal onFileProofAddress(QString address);
void MockWallet::requestFileProofAddress() {
    emit onFileProofAddress("lw7wfvw4f5gfboqvwwafbrqeryngxps4ygnskm7tdvy2e7bmmrgmecyd");
}

// Request all running swap trades.
// Check Signal: void onRequestSwapTrades(QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error);
void MockWallet::requestSwapTrades(QString cookie) {
    SwapInfo sw;
    sw.setData( "123.456", "0.00123", "BCH",
                         "XXXXX-XXXXXXXXXX-XXXXXX", "", 1603424302, "SellerCancelled", "State for this trade", "Action fro this trade",
                          1603454302, true, "mmmGZgkyaVvYnvkp6b4EXgsgN2UubNNZ1s", "" );

    emit onRequestSwapTrades(cookie, {sw}, "");
}

// Delete the swap trade
// Check Signal: void onDeleteSwapTrade(QString swapId, QString errMsg)
void MockWallet::deleteSwapTrade(QString swapId) {
    emit onDeleteSwapTrade(swapId, "");
}

// Create a new Swap trade deal.
// Check Signal: void onCreateNewSwapTrade(tag, dryRun, QString swapId, QString err);
void MockWallet::createNewSwapTrade(QString account,
                                    QVector<QString> outputs, // If defined, those outputs will be used to trade. They might belong to another trade, that if be fine.
                                    int min_confirmations, // minimum number of confimations
                                    QString mwcAmount, QString secAmount, QString secondary,
                                    QString redeemAddress,
                                    double secTxFee,
                                    bool sellerLockFirst,
                                    int messageExchangeTimeMinutes,
                                    int redeemTimeMinutes,
                                    int mwcConfirmationNumber,
                                    int secondaryConfirmationNumber,
                                    QString communicationMethod,
                                    QString communicationAddress,
                                    QString electrum_uri1,
                                    QString electrum_uri2,
                                    bool dryRun,
                                    QString tag,
                                    QString mkt_trade_tag,
                                    QVector<QString> params ) {
    Q_UNUSED(account)
    Q_UNUSED(outputs)
    Q_UNUSED(min_confirmations)
    Q_UNUSED(mwcAmount)
    Q_UNUSED(secAmount)
    Q_UNUSED(secondary)
    Q_UNUSED(redeemAddress)
    Q_UNUSED(secTxFee)
    Q_UNUSED(sellerLockFirst)
    Q_UNUSED(messageExchangeTimeMinutes)
    Q_UNUSED(redeemTimeMinutes)
    Q_UNUSED(mwcConfirmationNumber)
    Q_UNUSED(secondaryConfirmationNumber)
    Q_UNUSED(communicationMethod)
    Q_UNUSED(communicationAddress)
    Q_UNUSED(electrum_uri1)
    Q_UNUSED(electrum_uri2)
    Q_UNUSED(mkt_trade_tag)

    emit onCreateNewSwapTrade(tag, dryRun, params, "XXXXX-new-trade-id-XXXXX", "");
}

// Cancel the trade
// Check Signal: void onCancelSwapTrade(QString swapId, QString error);
void MockWallet::cancelSwapTrade(QString swapId) {
    emit onCancelSwapTrade(swapId, "");
}


// Request details about this trade.
// Check Signal: void onRequestTradeDetails( SwapTradeInfo swap,
//                            QVector<SwapExecutionPlanRecord> executionPlan,
//                            QString currentAction,
//                            QVector<SwapJournalMessage> tradeJournal,
//                            QString error,
//                            QString cookie );
void MockWallet::requestTradeDetails(QString swapId, bool waitForBackup1, QString cookie ) {
    Q_UNUSED(swapId)
    Q_UNUSED(waitForBackup1)
    SwapTradeInfo sw;
    emit onRequestTradeDetails( sw, {}, "current action", {}, "", cookie );
}

// Adjust swap stade values. params are optional
// Check Signal: onAdjustSwapData(QString swapId, QString adjustCmd, QString errMsg);
void MockWallet::adjustSwapData( const QString & swapId, QString call_tag,
                                 const QString &destinationMethod, const QString & destinationDest,
                                 const QString &secondaryAddress,
                                 const QString &secondaryFee,
                                 const QString &electrumUri1,
                                 const QString &tag ) {
    Q_UNUSED(swapId)

    Q_UNUSED(destinationMethod)
    Q_UNUSED(destinationDest)
    Q_UNUSED(secondaryAddress)
    Q_UNUSED(secondaryFee)
    Q_UNUSED(electrumUri1)
    Q_UNUSED(tag)

    emit onAdjustSwapData(swapId, call_tag, "");
}

// Perform a auto swap step for this trade.
// Check Signal: void onPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
//                       QString lastProcessError,
//                       QVector<SwapExecutionPlanRecord> executionPlan,
//                       QVector<SwapJournalMessage> tradeJournal,
//                       QString error );
void MockWallet::performAutoSwapStep( QString swapId, bool waitForBackup1 ) {
    Q_UNUSED(waitForBackup1)
    emit onPerformAutoSwapStep(swapId, "SellerSendingOffer", "current Action", "current State", "", {}, {}, "");
}

// Backup/export swap trade data file
// Check Signal: onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage)
void MockWallet::backupSwapTradeData(QString swapId, QString backupFileName) {
    emit onBackupSwapTradeData(swapId, backupFileName, "");
}

// Restore/import swap trade from the file
// Check Signal: onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);
void MockWallet::restoreSwapTradeData(QString filename) {
    onRestoreSwapTradeData("XXXX-swap-id-XXXXXXX", filename, "");
}

// Request proff address from http transaction
// apiSecret - if foreign API secret, optional. Normally it is empty
// Chack signal: onRequestRecieverWalletAddress(QString url, QString address, QString error)
void MockWallet::requestRecieverWalletAddress(QString url, QString apiSecret) {
    Q_UNUSED(url)
    Q_UNUSED(apiSecret)
    emit onRequestRecieverWalletAddress(url, "dhfkldgsfjfdkljhfdkjhkfdghgkfdhgkjh", "");
}

// Adjust trade state. It is dev support functionality, so no feedback will be provided.
// In case you need it, add the signal as usual
void MockWallet::adjustTradeState(QString swapId, QString newState) {
    Q_UNUSED(swapId)
    Q_UNUSED(newState)
};

}

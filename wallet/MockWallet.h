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

#ifndef MWC_QT_WALLET_MOCKWALLET_H
#define MWC_QT_WALLET_MOCKWALLET_H


#include "wallet.h"
#include <QObject>
#include <core/HodlStatus.h>
#include "../core/global.h"

namespace core {
class AppContext;
}

namespace wallet {

class MockWallet : public Wallet
{
Q_OBJECT

public:
    MockWallet(core::AppContext * appContext);
    virtual ~MockWallet() override;

    void setHodlStatus(core::HodlStatus * hodlStatus) {Q_UNUSED(hodlStatus);}


    virtual bool isWalletRunningAndLoggedIn() const override {return running;}

    // Return true if wallet is running
    virtual bool isRunning() override { return running;}


    // Check if walled need to be initialized or not. Will run statndalone app, wait for exit and return the result
    virtual bool checkWalletInitialized() override {return true;}

    virtual STARTED_MODE getStartedMode() override { return STARTED_MODE::NORMAL;}

    // ---- Wallet Init Phase
    virtual void start()  override { running=true; }

    // Create new wallet and generate a seed for it
    // Check signal: onNewSeed( seed [] )
    virtual void start2init(QString password)  override;

    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase:
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onRecoverResult(bool ok, QString newAddress );
    virtual void start2recover(const QVector<QString> & seed, QString password) override;

    // Check signal: onLoginResult(bool ok)
    virtual void loginWithPassword(QString password)  override;

    // Return true if wallet has password. Wallet might not have password if it was created manually.
    virtual bool hasPassword() const override {return true;}

    // Exit from the wallet. Expected that state machine will switch to Init state
    // syncCall - stop NOW. Caller suppose to understand what he is doing
    virtual void logout(bool syncCall) override { Q_UNUSED(syncCall); running=false;}

    // Confirm that user write the passphase
    // SYNC command
    virtual void confirmNewSeed() override {}

    // Current seed for runnign wallet
    // Check Signals: onGetSeed(QVector<QString> seed);
    virtual void getSeed(const QString & walletPassword) override;

    // Get last used password. Just don't export from DLL
    virtual QString getPasswordHash() override {return passwordHash;}

    //--------------- Listening

    // Checking if wallet is listening through services
    // return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
    virtual QPair<bool,bool> getListenerStatus() override { return QPair<bool,bool>(listener_mwcmqs, listener_keybase); }

    // return:  <mwcmq status>, <keybase status>.   true mean was started and need to be stopped, false - never started or was stopped
    virtual QPair<bool,bool> getListenerStartState() override { return QPair<bool,bool>(listener_mwcmqs, listener_keybase); }

    // Start listening through services
    // Check Signal: onStartListening
    virtual void listeningStart(bool startMq, bool startKb, bool initialStart)  override;

    // Stop listening through services
    // Check signal: onListeningStopResult
    virtual void listeningStop(bool stopMq, bool stopKb)  override;

    // Get latest Mwc MQ address that we see
    virtual QString getLastKnownMwcBoxAddress() override;

    // Get MWC box <address, index in the chain>
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
    virtual void getMwcBoxAddress()  override;

    // Change MWC box address to another from the chain. idx - index in the chain.
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
    virtual void changeMwcBoxAddress(int idx)  override;

    // Generate next box address
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
    virtual void nextBoxAddress()  override;

    // Request http(s) listening status.
    // bool - true is listening. Then next will be the address
    // bool - false, not listening. Then next will be error or empty if listening is not active.
    // Check signal: onHttpListeningStatus(bool listening, QString additionalInfo)
    virtual QPair<bool, QString> getHttpListeningStatus() const override {return QPair<bool, QString>(false, "");}

    // Return true if Tls is setted up for the wallet for http connections.
    virtual bool hasTls() const override { return false; }

    // -------------- Accounts

    // Get all accounts with balances. Expected that Wallet allways maintain them in a cache.
    // This info needed in many cases and we don't want spend time every time for that.
    virtual QVector<AccountInfo>  getWalletBalance(bool filterDeleted) const  override;

    // Get outputs that was collected for this wallet. Outputs should be ready with balances
    virtual const QMap<QString, QVector<wallet::WalletOutput> > & getwalletOutputs() const override;

    virtual QString getCurrentAccountName()  override {return currentAccount;}

    // Request sync (update_wallet_state) for the
    virtual void sync(bool showSyncProgress, bool enforce) override { Q_UNUSED(showSyncProgress); Q_UNUSED(enforce); }

    // Request Wallet balance update. It is a multistep operation
    // Check signal: onWalletBalanceUpdated
    //          onWalletBalanceProgress
    virtual void updateWalletBalance(bool enforceSync, bool showSyncProgress, bool skipSync=false) override;


    // Create another account, note no delete exist for accounts
    // Check Signal:  onAccountCreated
    virtual void createAccount( const QString & accountName )  override;

    // Switch to different account
    // Check Signal: onAccountSwitched
    virtual void switchAccount(const QString & accountName)  override;

    // Rename account
    // Check Signal: onAccountRenamed(bool success, QString errorMessage);
    virtual void renameAccount(const QString & oldName, const QString & newName)  override;

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onCheckResult(bool ok, QString errors );
    virtual void check(bool wait4listeners)  override;

    // Get current configuration of the wallet. will read from wallet713.toml file
    virtual WalletConfig getWalletConfig()  override;

    // Get configuration form the resource file.
    virtual WalletConfig getDefaultConfig()  override;

    // Update wallet config. Will update config and restart the mwc713.
    // Note!!! Caller is fully responsible for input validation. Normally mwc713 will start, but some problems might exist
    //          and caller suppose listen for them
    // If return true, expected that wallet will need to have password input.
    // Check signal: onConfigUpdate()
    virtual bool setWalletConfig( const WalletConfig & config, core::AppContext * appContext, node::MwcNode * mwcNode )  override;

    // Status of the node
    // return true if task was scheduled
    // Check Signal: onNodeSatatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections )
    virtual bool getNodeStatus() override;

    // -------------- Transactions

    // Set account that will receive the funds
    // Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
    virtual void setReceiveAccount(QString account)  override;

    // Cancel transaction
    // Check Signal:  onCancelTransacton
    virtual void cancelTransacton(int64_t transactionID)  override;

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    // Check Signal: onExportProof( bool success, QString fn, QString msg );
    virtual void generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName )  override;

    // Verify the proof for transaction
    // Check Signal: onVerifyProof( bool success, QString msg );
    virtual void verifyMwcBoxTransactionProof( QString proofFileName )  override;

    // Init send transaction with file output
    // Check signal:  onSendFile
    virtual void sendFile( const wallet::AccountInfo &account, int64_t coinNano, QString message, QString fileTx,
                           int inputConfirmationNumber, int changeOutputs,
                           const QStringList & outputs )  override;

    // Receive transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveFile
    virtual void receiveFile( QString fileTx, QString identifier = "")  override;

    // finalize transaction and broadcast it
    // Check signal:  onFinalizeFile
    virtual void finalizeFile( QString fileTxResponse, bool fluff )  override;

    // submit finalized transaction. Make sense for cold storage => online node operation
    // Check Signal: onSubmitFile(bool ok, String message)
    virtual void submitFile( QString fileTx ) override;

    // Airdrop special. Generating the next Pablic key for transaction
    // wallet713> getnextkey --amount 1000000
    // "Identifier(0300000000000000000000000600000000), PublicKey(38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79)"
    // Check Signal: onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor);
    virtual void getNextKey( int64_t amountNano, QString btcaddress, QString airDropAccPassword ) override;

    // Send some coins to address.
    // Before send, wallet always do the switch to account to make it active
    // Check signal:  onSend
    // coinNano == -1  - mean All
    virtual void sendTo( const wallet::AccountInfo &account, int64_t coinNano, const QString & address,
                         const QString & apiSecret,
                         QString message, int inputConfirmationNumber, int changeOutputs,
                         const QStringList & outputs, bool fluff )  override;

    // Show outputs for the wallet
    // Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> outputs)
    virtual void getOutputs(QString account, bool show_spent, bool enforceSync)  override;

    // Show all transactions for current account
    // Check Signal: onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions)
    virtual void getTransactions(QString account, bool enforceSync)  override;

    // get Extended info for specific transaction
    // Check Signal: onTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages )
    virtual void getTransactionById(QString account, int64_t txIdx )  override;

    // Read all transactions for all accounts. Might take time...
    // Check Signal: onAllTransactions( QVector<WalletTransaction> Transactions)
    virtual void getAllTransactions() override;

    // Get root public key with signed message. Message is optional, can be empty
    // Check Signal: onRootPublicKey( QString rootPubKey, QString message, QString signature )
    virtual void getRootPublicKey( QString message2sign ) override;


private:
    core::AppContext * appContext; // app context to store current account name

    QString passwordHash;
    bool running = false;
    bool listener_mwcmqs = false;
    bool listener_keybase = false;

    QString mwcAddress; // Address from mwc listener

    QVector<AccountInfo> accountInfo;
    QString currentAccount = "default";
};

}


#endif //MWC_QT_WALLET_MOCKWALLET_H

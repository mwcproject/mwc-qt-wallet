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

#ifndef MWC713_H
#define MWC713_H

#include "wallet.h"
#include <QObject>
#include <QProcess>
#include <core/HodlStatus.h>
#include "../core/global.h"

namespace tries {
    class Mwc713InputParser;
}

namespace core {
class AppContext;
}

namespace wallet {

class Mwc713EventManager;

class MWC713 : public Wallet
{
    Q_OBJECT
public:

    // Read config from the file. By default read from config::getMwc713conf()
    static WalletConfig readWalletConfig(QString source = "");
    // Save config into config::getMwc713conf()
    // !!! Note !!!! Also it start/stop local mwcNode if it is needed by setting. Stop can take for a while
    static bool saveWalletConfig(const WalletConfig & config, core::AppContext * appContext, node::MwcNode * mwcNode);

public:
    MWC713(QString mwc713path, QString mwc713configPath, core::AppContext * appContext);
    virtual ~MWC713() override;

    void setHodlStatus(core::HodlStatus * _hodlStatus) {hodlStatus = _hodlStatus; Q_ASSERT(hodlStatus);}


    // Return true if wallet is running
    virtual bool isRunning() override {return mwc713process!= nullptr;}


    // Check if waaled need to be initialized or not. Will run statndalone app, wait for exit and return the result
    // Call might take few seconds
    virtual bool checkWalletInitialized() override;

    virtual STARTED_MODE getStartedMode() override { if (mwc713process==nullptr) {return STARTED_MODE::OFFLINE;} return startedMode;}

    // ---- Wallet Init Phase
    virtual void start(bool loginWithLastKnownPassword)  override;
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
    virtual bool hasPassword() const override;

    // Exit from the wallet. Expected that state machine will switch to Init state
    // syncCall - stop NOW. Caller suppose to understand what he is doing
    virtual void logout(bool syncCall) override;

    // Confirm that user write the passphase
    // SYNC command
    virtual void confirmNewSeed()  override;

    // Current seed for runnign wallet
    // Check Signals: onGetSeed(QVector<QString> seed);
    virtual void getSeed()  override;

    // Get last used password. Just don't export from DLL
    virtual QString getPassword() override;

    //--------------- Listening

    // Checking if wallet is listening through services
    // return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
    virtual QPair<bool,bool> getListenerStatus() override;
    // return:  <mwcmq status>, <keybase status>.   true mean was started and need to be stopped, false - never started or was stopped
    virtual QPair<bool,bool> getListenerStartState() override;

    // Start listening through services
    virtual void listeningStart(bool startMq, bool startKb, bool initialStart)  override;
    // Check Signal: onStartListening

    // Stop listening through services
    virtual void listeningStop(bool stopMq, bool stopKb)  override;
    // Check signal: onListeningStopResult

    // Get latest Mwc MQ address that we see
    virtual QString getLastKnownMwcBoxAddress()  override;

    // Get MWC box <address, index in the chain>
    virtual void getMwcBoxAddress()  override;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Change MWC box address to another from the chain. idx - index in the chain.
    virtual void changeMwcBoxAddress(int idx)  override;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Generate next box address
    virtual void nextBoxAddress()  override;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Request http(s) listening status.
    // bool - true is listening. Then next will be the address
    // bool - false, not listening. Then next will be error or empty if listening is not active.
    virtual QPair<bool, QString> getHttpListeningStatus() const override;
    // Check signal: onHttpListeningStatus(bool listening, QString additionalInfo)

    // Return true if Tls is setted up for the wallet for http connections.
    virtual bool hasTls() const override;

    // -------------- Accounts

    // Get all accounts with balances. Expected that Wallet allways maintain them in a cache.
    // This info needed in many cases and we don't want spend time every time for that.
    virtual QVector<AccountInfo>  getWalletBalance(bool filterDeleted = true) const  override;

    virtual QString getCurrentAccountName()  override {return currentAccount;}

    // Request sync (update_wallet_state) for the
    virtual void sync(bool showSyncProgress, bool enforce) override;

    // Request Wallet balance update. It is a multistep operation
    virtual void updateWalletBalance(bool enforceSync, bool showSyncProgress, bool skipSync=false) override;
    // Check signal: onWalletBalanceUpdated
    //          onWalletBalanceProgress


    // Create another account, note no delete exist for accounts
    virtual void createAccount( const QString & accountName )  override;
    // Check Signal:  onAccountCreated

    // Switch to different account
    virtual void switchAccount(const QString & accountName)  override;
    // Check Signal: onAccountSwitched

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
    virtual void finalizeFile( QString fileTxResponse )  override;

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
                         const QStringList & outputs )  override;

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

public:
    // Feed the command to mwc713 process
    void executeMwc713command( QString cmd, QString shadowStr);

    virtual bool isWalletRunningAndLoggedIn() const override { return ! (mwc713process== nullptr || eventCollector== nullptr || startedMode != STARTED_MODE::NORMAL || loggedIn==false ); }

public:
    // stop mwc713 process nicely
    void processStop(bool exitNicely);

    void setLoginResult(bool ok);

    void setGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor);

    // Wallet init status
   // enum INIT_STATUS {NONE, NEED_PASSWORD, NEED_SEED, WRONG_PASSWORD, READY };
  //  void setInitStatus( INIT_STATUS  initStatus );

    void setMwcAddress( QString mwcAddress ); // Set active MWC address. Listener might be offline
    void setMwcAddressWithIndex( QString mwcAddress, int idx );

    void setNewSeed( QVector<QString> seed );

    void setGettedSeed( QVector<QString> seed );

    void setListeningStartResults( bool mqTry, bool kbTry, // what we try to start
            QStringList errorMessages, bool initialStart );

    void setListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                                QStringList errorMessages );

    // tid - thread ID that is responsible for listening. mwc713 can do start/stop async. tid will be used to find who is listening...
    void setMwcMqListeningStatus(bool online, QString tid, bool startStopEvents); // Start stop event are major, they can change active tid
    void setKeybaseListeningStatus(bool online);

    // info: if online  - Address, offlone - Error message or empty.
    void setHttpListeningStatus(bool online, QString info);

    void setRecoveryResults( bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages );
    void setRecoveryProgress( int64_t progress, int64_t limit );

    // Update account feedback
    void updateAccountList( QVector<QString> accounts );
    void updateAccountProgress(int accountIdx, int totalAccounts);
    void updateAccountFinalize(QString prevCurrentAccount);
    void createNewAccount( QString newAccountName );
    void switchToAccount( QString switchAccountName, bool makeAccountCurrent );

    void updateRenameAccount(const QString & oldName, const QString & newName, bool createSimulation,
                             bool success, QString errorMessage);

    void infoResults( QString currentAccountName, int64_t height,
                      int64_t totalConfirmedNano, int64_t waitingConfNano,
                      int64_t waitingFinalizetinNano, int64_t lockedNano,
                      int64_t spendableNano,
                      bool mwcServerBroken );

    void setSendResults(bool success, QStringList errors, QString address, int64_t txid, QString slate);

    void reportSlateSendTo( QString slate, QString mwc, QString sendAddr );
    void reportSlateSendBack( QString slate, QString sendAddr );
    void reportSlateReceivedFrom( QString slate, QString mwc, QString fromAddr, QString message );
    void reportSlateReceivedBack( QString slate, QString mwc, QString fromAddr );
    void reportSlateFinalized( QString slate );

    void setSendFileResult( bool success, QStringList errors, QString fileName );
    void setReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
    void setFinalizeFile( bool success, QStringList errors, QString fileName );
    void setSubmitFile(bool success, QString message, QString fileName);

    // Transactions
    void setTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions);

    void setTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages );

    // Outputs results
    void setOutputs( QString account, int64_t height, QVector<WalletOutput> outputs);

    void setExportProofResults( bool success, QString fn, QString msg );
    void setVerifyProofResults( bool success, QString fn, QString msg );

    void setTransCancelResult( bool success, int64_t transId, QString errMsg );

    void setSetReceiveAccount( bool ok, QString accountOrMessage );

    void setCheckResult(bool ok, QString errors);

    void setNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

    void setRootPublicKey( bool success, QString errMsg,
            QString rootPubKey, QString message, QString signature );

    void notifyListenerMqCollision();
    void notifyMqFailedToStart();

    //-------------
    void processAllTransactionsStart();
    void processAllTransactionsAppend(const QVector<WalletTransaction> & trVector);
    void processAllTransactionsEnd();

    // -----------------
    void updateSyncProgress(double progressPercent);

    void updateSyncAsDone();

    Mwc713EventManager * getEventCollector() {return eventCollector;}
    core::HodlStatus *   getHodlStatus() {return hodlStatus;}

private:


    void mwc713connect(QProcess * process, bool trackProcessExit);
    void mwc713disconnect();

    // Update acc value at collection accounts. If account is not founf, we can add it (addIfNotFound) or skip
    void updateAccountInfo( const AccountInfo & acc, QVector<AccountInfo> & accounts, bool addIfNotFound ) const;

    // pass - provide password through env variable. If pass empty - nothing will be done
    // envVariables - environment variables (key/value). Must be in pairs.
    // paramsPlus - additional parameters for the process
    QProcess * initMwc713process( const QStringList & envVariables, const QStringList & paramsPlus, bool trackProcessExit = true );

    // Reset data as wallet not started yet
    void resetData(STARTED_MODE startedMode);

private slots:
    // mwc713 Process IOs
    void	mwc713errorOccurred(QProcess::ProcessError error);
    void	mwc713finished(int exitCode, QProcess::ExitStatus exitStatus);
    void	mwc713readyReadStandardError();
    void	mwc713readyReadStandardOutput();

    void    restartMQsListener();

private:

private:
    core::AppContext * appContext; // app context to store current account name
    core::HodlStatus * hodlStatus = nullptr;

    QString mwc713Path; // path to the backed binary
    QString mwc713configPath; // config file for mwc713
    QProcess * mwc713process = nullptr;
    tries::Mwc713InputParser * inputParser = nullptr; // Parser will generate bunch of signals that wallet will listem on
    const int outputsLinesBufferSize = 15;
    QList<QString> outputsLines; // Last few output lines. Will print in case of the crash

    STARTED_MODE startedMode = STARTED_MODE::OFFLINE;
    bool   loggedIn = false; // Make sence for startedMode NORMAL. True if login was successfull

    Mwc713EventManager * eventCollector = nullptr;

    // Stages (flags) of the wallet
    //InitWalletStatus initStatus = InitWalletStatus::NONE;

    QString mwcAddress; // Address from mwc listener

    // listening statuses
    bool mwcMqOnline = false;
    bool keybaseOnline = false;
    bool mwcMqStarted = false;
    bool keybaseStarted = false;
    // MWC MQS will try to start forever.
    bool mwcMqStartRequested = false;

    bool httpOnline = false;
    QString httpInfo = "";
    bool hasHttpTls = false;

    QString activeMwcMqsTid; // MQS can be managed by many thredas, but only last started is active

    // Connections to mwc713process
    QVector< QMetaObject::Connection > mwc713connections; // open connection to mwc713

    // Accounts with balances info
    QVector<AccountInfo> accountInfo;
    QString currentAccount = "default"; // Keep current account by name. It fit better to mwc713 interactions.

    int64_t lastSyncTime = 0;
private:
    // Temprary values, local values for states
    QString walletPassword;

    QVector<AccountInfo> collectedAccountInfo;

    QVector<WalletTransaction> collectedTransactions;

    int64_t walletStartTime = 0;
    QString commandLine;
};

}

#endif // MWC713_H

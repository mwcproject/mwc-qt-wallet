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
#include "../core/global.h"
#include <QMap>

namespace tries {
    class Mwc713InputParser;
}

namespace core {
class AppContext;
}

namespace wallet {

class Mwc713EventManager;
class Mwc713Task;

class MWC713 : public Wallet
{
    Q_OBJECT
public:

    // Read config from the file. By default read from config::getMwc713conf()
    static WalletConfig readWalletConfig(QString source = "");
    // Save config into config::getMwc713conf()
    // !!! Note !!!! Also it start/stop local mwcNode if it is needed by setting. Stop can take for a while
    static bool saveWalletConfig(const WalletConfig & config, core::AppContext * appContext, node::MwcNode * mwcNode, bool canStartNode);

public:
    MWC713(QString mwc713path, QString mwc713configPath, core::AppContext * appContext, node::MwcNode * mwcNode);
    virtual ~MWC713() override;

    // Return true if wallet is running
    virtual bool isRunning() override {return mwc713process!= nullptr;}


    // Check if walled need to be initialized or not. Will run statndalone app, wait for exit and return the result
    // Call might take few seconds
    virtual bool checkWalletInitialized(bool hasSeed) override;

    virtual STARTED_MODE getStartedMode() override { if (mwc713process==nullptr) {return STARTED_MODE::OFFLINE;} return startedMode;}

    // ---- Wallet Init Phase
    virtual void start()  override;
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
    virtual void getSeed(const QString & walletPassword)  override;

    // Get last used password. Just don't export from DLL
    virtual QString getPasswordHash() override;

    //--------------- Listening

    // Checking if wallet is listening through services
    virtual ListenerStatus getListenerStatus() override;
    virtual ListenerStatus getListenerStartState() override;

    // Start listening through services
    virtual void listeningStart(bool startMq, bool startTor, bool initialStart)  override;
    // Check Signal: onStartListening

    // Stop listening through services
    virtual void listeningStop(bool stopMq, bool stopTor)  override;
    // Check signal: onListeningStopResult

    // Get latest Mwc MQ address that we see
    virtual QString getMqsAddress() override;

    // Get latest Tor address that we see
    virtual QString getTorAddress()  override;

    // Request proof address for files
    virtual void requestFileProofAddress() override;

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

    // Get outputs that was collected for this wallet. Outputs should be ready with balances
    virtual const QMap<QString, QVector<wallet::WalletOutput> > & getwalletOutputs() const override {return walletOutputs;}

    virtual QString getCurrentAccountName()  override {return currentAccount;}

    // Request Wallet balance update. It is a multistep operation
    virtual void updateWalletBalance(bool enforceSync, bool showSyncProgress, bool skipSync=false) override;
    // Check signal: onWalletBalanceUpdated
    //          onWalletBalanceProgress


    // Create another account, note no delete exist for accounts
    virtual void createAccount( const QString & accountName )  override;
    // Check Signal:  onAccountCreated

    // Switch to different account, no feedback is expected
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
    virtual const WalletConfig & getWalletConfig()  override;

    // Get configuration form the resource file.
    virtual const WalletConfig & getDefaultConfig()  override;

    // Update wallet config. Will update config and restart the mwc713.
    // Note!!! Caller is fully responsible for input validation. Normally mwc713 will start, but some problems might exist
    //          and caller suppose listen for them
    // If return true, expected that wallet will need to have password input.
    // Check signal: onConfigUpdate()
    virtual bool setWalletConfig( const WalletConfig & config, bool canStartNode )  override;

    // Status of the node
    // return true if task was scheduled
    // Check Signal: onNodeSatatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections )
    virtual bool getNodeStatus() override;

    // -------------- Transactions

    // Set account that will receive the funds
    // Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
    virtual void setReceiveAccount(QString account)  override;
    virtual QString getReceiveAccount() override {return recieveAccount;}

    // Cancel transaction
    // Check Signal:  onCancelTransacton
    virtual void cancelTransacton(QString account, int64_t txIdx)  override;

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    // Check Signal: onExportProof( bool success, QString fn, QString msg );
    virtual void generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName )  override;

    // Verify the proof for transaction
    // Check Signal: onVerifyProof( bool success, QString msg );
    virtual void verifyMwcBoxTransactionProof( QString proofFileName )  override;

    // Init send transaction with file output
    // Check signal:  onSendFile
    virtual void sendFile( const QString &account, int64_t coinNano, QString message, QString fileTx,
            int inputConfirmationNumber, int changeOutputs,
            const QStringList & outputs, int ttl_blocks, bool generateProof )  override;
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
    virtual void sendTo( const QString &account, int64_t coinNano, const QString & address,
                         const QString & apiSecret,
                         QString message, int inputConfirmationNumber, int changeOutputs,
                         const QStringList & outputs, bool fluff, int ttl_blocks, bool generateProof, QString expectedproofAddress )  override;

    // Show outputs for the wallet
    // Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> outputs)
    virtual void getOutputs(QString account, bool show_spent, bool enforceSync)  override;

    // Show all transactions for current account
    // Check Signal: onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions)
    virtual void getTransactions(QString account, bool enforceSync)  override;

    // get Extended info for specific transaction
    // Check Signal: onTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages )
    virtual void getTransactionById(QString account, int64_t txIdx )  override;

    // Get root public key with signed message. Message is optional, can be empty
    // Check Signal: onRootPublicKey( QString rootPubKey, QString message, QString signature )
    virtual void getRootPublicKey( QString message2sign ) override;

    // Repost a transaction. Optionally fluff the transaction. index is the tx_index in the tx_log.
    // Check Signal: onRepost(QString error)
    virtual void repost(QString account, int index, bool fluff) override;

    // ---------------- Swaps -------------

    // Request all running swap trades.
    // Check Signal: void onRequestSwapTrades(QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error);
    virtual void requestSwapTrades(QString cookie) override;

    // Delete the swap trade
    // Check Signal: void onDeleteSwapTrade(QString swapId, QString errMsg)
    virtual void deleteSwapTrade(QString swapId) override;

    // Create a new Swap trade deal.
    // Check Signal: void onCreateNewSwapTrade(tag, dryRun, QVector<QString> params, QString swapId, QString err);
    virtual void createNewSwapTrade(QString account,
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
                                    QVector<QString> params ) override;

    // Cancel the trade
    // Check Signal: void onCancelSwapTrade(QString swapId, QString error);
    virtual void cancelSwapTrade(QString swapId) override;

    // Request details about this trade.
    // Check Signal: void onRequestTradeDetails( SwapTradeInfo swap,
    //                            QVector<SwapExecutionPlanRecord> executionPlan,
    //                            QString currentAction,
    //                            QVector<SwapJournalMessage> tradeJournal,
    //                            QString error );
    virtual void requestTradeDetails(QString swapId, bool waitForBackup1) override;

    // Adjust swap stade values. params are optional
    // Check Signal: onAdjustSwapData(QString swapId, QString adjustCmd, QString errMsg);
    virtual void adjustSwapData( QString swapId, QString adjustCmd, QString param1 = "", QString param2 = "" ) override;

    // Perform a auto swap step for this trade.
    // Check Signal: void onPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
    //                       QString lastProcessError,
    //                       QVector<SwapExecutionPlanRecord> executionPlan,
    //                       QVector<SwapJournalMessage> tradeJournal,
    //                       QString error );
    virtual void performAutoSwapStep( QString swapId, bool waitForBackup1 ) override;

    // Backup/export swap trade data file
    // Check Signal: onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage)
    virtual void backupSwapTradeData(QString swapId, QString backupFileName) override;

    // Restore/import swap trade from the file
    // Check Signal: onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);
    virtual void restoreSwapTradeData(QString filename) override;

    // Request proof address for http transaction
    // Check signal: onRequestRecieverWalletAddress(QString url, QString address, QString error)
    virtual void requestRecieverWalletAddress(QString url, QString apiSecret) override;

    // Adjust trade state. It is dev support functionality, so no feedback will be provided.
    // In case you need it, add the signal as usuall
    virtual void adjustTradeState(QString swapId, QString newState) override;
public:
    // launch exit command.
    void launchExitCommand();

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

    void setTorAddress( QString torAddress ); // Set active Tor address.
    void setMwcAddress( QString mwcAddress ); // Set active MWC address. Listener might be offline
    void setMwcAddressWithIndex( QString mwcAddress, int idx );
    void setFileProofAddress( QString address );

    void setNewSeed( QVector<QString> seed );

    void setGettedSeed( QVector<QString> seed );

    void setListeningStartResults( bool mqTry, bool torTry, // what we try to start
            QStringList errorMessages, bool initialStart );

    void setListeningStopResult(bool mqTry, bool torTry, // what we try to stop
                                QStringList errorMessages );

    // tid - thread ID that is responsible for listening. mwc713 can do start/stop async. tid will be used to find who is listening...
    void setMwcMqListeningStatus(bool online, QString tid, bool startStopEvents); // Start stop event are major, they can change active tid
    void setTorListeningStatus(bool online);

    // info: if online  - Address, offlone - Error message or empty.
    void setHttpListeningStatus(bool online, QString info);

    void setRecoveryResults( bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages );
    void setRecoveryProgress( int64_t progress, int64_t limit );

    // Update account feedback
    void updateAccountList( QVector<QString> accounts );
    void updateAccountProgress(int accountIdx, int totalAccounts);
    void updateAccountFinalize();
    void createNewAccount( QString newAccountName );

    void updateRenameAccount(const QString & oldName, const QString & newName, bool createSimulation,
                             bool success, QString errorMessage);

    void infoResults( QString currentAccountName, int64_t height,
                      int64_t totalConfirmedNano, int64_t waitingConfNano,
                      int64_t waitingFinalizetinNano, int64_t lockedNano,
                      int64_t spendableNano,
                      bool mwcServerBroken );

    void setSendResults(bool success, QStringList errors, QString address, int64_t txid, QString slate, QString mwc);
    void reportSlateReceivedFrom( QString slate, QString mwc, QString fromAddr, QString message );

    void setSendFileResult( bool success, QStringList errors, QString fileName );
    void setReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
    void setFinalizeFile( bool success, QStringList errors, QString fileName );
    void setSubmitFile(bool success, QString message, QString fileName);

    // Transactions
    void setTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions);

    void setTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages );

    // Outputs results
    void setOutputs( QString account, bool show_spent, int64_t height, QVector<WalletOutput> outputs);

    void setWalletOutputs( const QString & account, const QVector<WalletOutput> & outputs);

    void setExportProofResults( bool success, QString fn, QString msg );
    void setVerifyProofResults( bool success, QString fn, QString msg );

    void setTransCancelResult( bool success, const QString & account, int64_t transId, QString errMsg );

    void setSetReceiveAccount( bool ok, QString accountOrMessage );

    void setCheckResult(bool ok, QString errors);

    void setNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

    void setRootPublicKey( bool success, QString errMsg,
            QString rootPubKey, QString message, QString signature );

    void notifyListenerMqCollision();
    void notifyMqFailedToStart();

    // -----------------
    void updateSyncProgress(double progressPercent);

    void updateSyncAsDone();

    Mwc713EventManager * getEventCollector() {return eventCollector;}


    void setRequestSwapTrades(QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error);
    void setDeleteSwapTrade(QString swapId, QString errMsg);
    void setCreateNewSwapTrade( QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg );
    void setCancelSwapTrade(QString swapId, QString errMsg);
    void setRequestTradeDetails( SwapTradeInfo swap,
                                QVector<SwapExecutionPlanRecord> executionPlan,
                                QString currentAction,
                                QVector<SwapJournalMessage> tradeJournal,
                                QString error );
    void setAdjustSwapData(QString swapId, QString adjustCmd, QString errMsg);

    void setPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                               QString lastProcessError,
                               QVector<SwapExecutionPlanRecord> executionPlan,
                               QVector<SwapJournalMessage> tradeJournal,
                               QString error );

    // Notificaiton that nee Swap trade offer was recieved.
    void notifyAboutNewSwapTrade(QString currency, QString swapId);

    void setBackupSwapTradeData(QString swapId, QString backupFileName, QString errorMessage);

    void setRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);

    void setRequestRecieverWalletAddress(QString url, QString address, QString error);

    void setRepost(int txIdx, QString err);
private:
    // Request sync (update_wallet_state) if it is not at the task Q.
    QVector<QPair<Mwc713Task*,int64_t>> create_sync_if_need(bool showSyncProgress, bool enforce);

    void mwc713connect(QProcess * process, bool trackProcessExit);
    void mwc713disconnect();

    // pass - provide password through env variable. If pass empty - nothing will be done
    // envVariables - environment variables (key/value). Must be in pairs.
    // paramsPlus - additional parameters for the process
    QProcess * initMwc713process( const QStringList & envVariables, const QStringList & paramsPlus, bool trackProcessExit = true );

    // Reset data as wallet not started yet
    void resetData(STARTED_MODE startedMode);

    // Updating config according to what is stored at the path
    bool updateWalletConfig(const QString & path, bool canStartNode);

private slots:
    // mwc713 Process IOs
    void	mwc713errorOccurred(QProcess::ProcessError error);
    void	mwc713finished(int exitCode, QProcess::ExitStatus exitStatus);
    void	mwc713readyReadStandardError();
    void	mwc713readyReadStandardOutput();

    void    restartMQsListener();

    void    onOutputLockChanged(QString commit);
private:

    // process accountInfoNoLocks, apply locked outputs
    QVector<AccountInfo> applyOutputLocksToBalance() const;

private:
    core::AppContext * appContext = nullptr; // app context to store current account name
    node::MwcNode * mwcNode = nullptr;

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
    QString torAddress; // Address from Tor listener

    // listening statuses
    bool mwcMqOnline = false;
    bool torOnline = false;
    bool mwcMqStarted = false;
    bool torStarted = false;
    // MWC MQS will try to start forever.
    bool mwcMqStartRequested = false;

    bool httpOnline = false;
    QString httpInfo = "";
    bool hasHttpTls = false;

    QString activeMwcMqsTid; // MQS can be managed by many thredas, but only last started is active

    // Connections to mwc713process
    QVector< QMetaObject::Connection > mwc713connections; // open connection to mwc713

    // Accounts with balances info
    QVector<AccountInfo> accountInfoNoLocks;
    QString currentAccount = "default"; // Keep current account by name. It fit better to mwc713 interactions.
    QString recieveAccount = "default";

    QMap<QString, QVector<wallet::WalletOutput> > walletOutputs; // Available outputs from this wallet. Key: account name, value outputs for this account

    int64_t lastSyncTime = 0;

    WalletConfig currentConfig;
    WalletConfig defaultConfig;
private:
    // Temprary values, local values for states
    QString walletPasswordHash;

    QVector<AccountInfo> collectedAccountInfo;

    int64_t walletStartTime = 0;
    QString commandLine;
};

}

#endif // MWC713_H

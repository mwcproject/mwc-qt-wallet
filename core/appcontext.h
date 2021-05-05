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

#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QMap>
#include "../state/state.h"
#include "../state/m_airdrop.h"
#include "../wallet/wallet.h"
#include "../core/Config.h"
#include "../bridge/wnd/g_send_b.h"
#include <QDebug>
#include <QHash>

class QAction;

namespace core {

struct SendCoinsParams {
    int inputConfirmationNumber;
    int changeOutputs;

    // Expected to be default values
    SendCoinsParams() :
            inputConfirmationNumber(10), changeOutputs(1) {}

    SendCoinsParams( int _inputConfirmationNumber, int _changeOutputs) :
            inputConfirmationNumber(_inputConfirmationNumber), changeOutputs(_changeOutputs) {}

    void setData(int _inputConfirmationNumber, int _changeOutputs) {
        inputConfirmationNumber = _inputConfirmationNumber;
        changeOutputs = _changeOutputs;
    }

    SendCoinsParams(const SendCoinsParams & o) = default;
    SendCoinsParams & operator = (const SendCoinsParams & o) = default;

    bool operator == (const SendCoinsParams & o ) {return inputConfirmationNumber==o.inputConfirmationNumber && changeOutputs==o.changeOutputs;}

    void saveData(QDataStream & out) const;
    bool loadData(QDataStream & in);
};

struct ContactRecord {
    QString name;
    QString address;
    QString pub_key;

    ContactRecord() = default;
    ContactRecord(const ContactRecord & other) = default;
    ContactRecord(const QString & n, const QString & a);

    bool operator ==(const ContactRecord & o) {return name == o.name && address == o.address;}

    void setData(QString name,
                 QString address);

    void saveData( QDataStream & out) const;
    bool loadData( QDataStream & in);
};

// State that applicable to all application.
// Support signal for changes about Locked output
class AppContext : public QObject
{
    Q_OBJECT
public:
    AppContext();
    ~AppContext();

    // add new key/value
    template <class T>
    void pushCookie(QString key, T value);

    // remove and return
    template <class T>
    T pullCookie(QString key);

    // get is exist, don't clean up
    template <class T>
    T getCookie(QString key);


    void initGuiScale(const double _initScaleValue) { initScaleValue = _initScaleValue; }
    double getInitGuiScale() const {return initScaleValue;}

    double getGuiScale() const;
    void setGuiScale(double scale) { guiScale = scale; }

    state::STATE getActiveWndState() const {return activeWndState;}
    void setActiveWndState(state::STATE  state) {activeWndState=state;}

    // Send coins params.
    SendCoinsParams getSendCoinsParams() const {return sendCoinsParams;}
    void setSendCoinsParams(SendCoinsParams params) { sendCoinsParams=params; }

    // Get last path state. Default: Home dir
    QString getPathFor( QString name, bool returnEmptyForNew = false ) const;
    // update path state
    void updatePathFor( QString name, QString path );

    // IO for Int vectors
    QVector<int> getIntVectorFor( QString name ) const;
    void updateIntVectorFor( QString name, const QVector<int> & data );

    QString getReceiveAccount(const QString & walletDataDir) const { return receiveAccount.value(walletDataDir, "default");}
    void setReceiveAccount(const QString & walletDataDir, const QString & account) {receiveAccount.insert(walletDataDir, account);}

    QString getCurrentAccountName(const QString & walletDataDir) const {return currentAccountName.value(walletDataDir, "default");};
    void setCurrentAccountName(const QString & walletDataDir, const QString & currentAccount) {currentAccountName.insert(walletDataDir, currentAccount);}

    // -------------- Contacts
    // Get the contacts
    QVector<ContactRecord> getContacts() const {return contactList;}
    // Add s new contact
    QPair<bool, QString> addContact( const ContactRecord & contact );
    // Remove contact. return false if not found
    QPair<bool, QString> deleteContact( const ContactRecord & contact );
    // Update contact
    QPair<bool, QString> updateContact( const ContactRecord & prevValue, const ContactRecord & newValue );

    // ----- Logs --------
    bool isLogsEnabled() const {return logsEnabled;}
    void setLogsEnabled(bool enabled);

    // ----- Auto Start MQS --------
    bool isAutoStartMQSEnabled() const {return autoStartMQSEnabled;}
    void setAutoStartMQSEnabled(bool enabled);

    // ----- Auto Start Tor ------
    bool isAutoStartTorEnabled() const {return autoStartTorEnabled;}
    void setAutoStartTorEnabled(bool enabled);
    // Not will be started with TOR if it is online node or Tor autostart enabled
    bool useTorForNode() const;

    // ----- Outputs: All/Unspent
    bool isShowOutputAll() const {return showOutputAll;}
    void setShowOutputAll(bool all);

    wallet::MwcNodeConnection getNodeConnection( const QString & network );
    void updateMwcNodeConnection( const QString & network, const wallet::MwcNodeConnection & connection );

    // HODL registration time.
    int64_t getHodlRegistrationTime(const QString & hash) const;
    void    setHodlRegistrationTime(const QString & hash, int64_t time);

    QString getNote(const QString& key);
    void updateNote(const QString& key, const QString& note);
    void deleteNote(const QString& key);

    // Outputs can be locked from spending.
    bool isLockOutputEnabled() const {return lockOutputEnabled;}
    // Return lock flag and output ID
    QPair<bool, QString> isLockedOutputs(const QString & output) const;
    void setLockOutputEnabled(bool enabled);
    void setLockedOutput(const QString & output, bool lock, QString Id);
    void unlockOutputsById(QString id);
    QVector<QString> getLockedOutputsById(QString id) const;

    // Fluff transactions
    bool isFluffSet() const { return fluffTransactions; }
    void setFluff(bool fluffSetting);

    // Transaction note migration
    bool hasTxnNotesToMigrate();
    QStringList getTxnNotesToMigrate(QString walletId, QString accountId);
    void migrateTxnNote(QString walletId, QString accountId, QString txIdx, QString txUuid);

    // Wallet instances. Return instances paths that are valid and current selected index
    QPair<QVector<QString>, int> getWalletInstances(bool hasSeed) const;
    // Return path to current wallet instance. Expected that at least 1 instance does exist
    QString getCurrentWalletInstance(bool hasSeed) const;
    void setCurrentWalletInstance(const QString & path);
    // Add new instance and make it active
    void addNewInstance(const QString & instance);

    // Check if inline node running the main network
    bool isOnlineNodeRunsMainNetwork() const;
    void setOnlineNodeRunsMainNetwork(bool isMainNet);

    // Generate proof for all send transactions.
    bool getGenerateProof() const {return generateProof;}
    void setGenerateProof(bool proof);

    // Display of status messages using notification windows
    bool getNotificationWindowsEnabled() { return notificationWindowsEnabled; }
    void setNotificationWindowsEnabled(bool enable);

    // Index of the open tab at the swap page.
    int getSwapTabSelection() const {return swapTabSelection;}
    void setSwapTabSelection(int sel) { swapTabSelection=sel;}

    // Backup dir for swap trades.
    QString getSwapBackupDir() const {return swapBackupDir;}
    void setSwapBackupDir(QString backupDir);

    QString getLastUsedSwapCurrency() const {return lastUsedSwapCurrency;}
    void    setLastUsedSwapCurrency(const QString & currency) {lastUsedSwapCurrency = currency;}

    // return 0 for the first call.
    int  getSwapBackStatus(const QString & swapId) const;
    void setSwapBackStatus(const QString & swapId, int status);

    int getMaxBackupStatus(QString swapId, int status);

    // Check fir accepted trades (we don't want ask to acceptance twice. The workflow can return back)
    bool isTradeAccepted(const QString & swapId) const;
    void setTradeAcceptedFlag(const QString & swapId, bool accepted);

    bool getNoTorForEmbeddedNode() const {return noTorForEmbeddedNode;}
    void setNoTorForEmbeddedNode(bool noTor);

    bridge::SEND_SELECTED_METHOD getSendMethod() const {return sendMethod;}
    void setSendMethod(bridge::SEND_SELECTED_METHOD sendMethod);

    bool getSendLockOutput() const {return sendLockOutput; }
    void setSendLockOutput(bool lock);

    double getMktFeeReservedAmount() const {return mktFeeReservedAmount;}
    void setMktFeeReserveAmount(const double &reservedAmount);

    QString getMktFeeDepositAccount() const {return mktFeeDepositAccount;}
    void setMktFeeDepositAccount(QString accountName);

    double getMktFeeLevel() const {return mktFeeLevel;}
    void setMktFeeLevel(const double &feeLevel);

    int getMktPlaceSelectedBtn() const {return mktPlaceSelectedBtn;}
    void setMktPlaceSelectedBtn(int btn);

    double getSwapMktMinFeeLevel() const {return mktPlaceMinFeeLevel;}
    int getSwapMktSelling() const {return mktPlaceSelling;}
    QString getSwapMktCurrency() const {return mktPlaceCurrency;}

    void setSwapMktMinFeeLevel(double fee);
    void setSwapMktSelling(int value);
    void setSwapMktCurrency(QString currency);

private:
signals:
    void onOutputLockChanged(QString commit);

private:
    bool loadData();
    bool loadDataImpl();

    void saveData() const;

    void loadNotesData();
    void saveNotesData() const;
    void migrateOutputNotes();

private:
    // 16 bit hash from the password. Can be used for the password verification
    // Don't use many bits because we don't want it be much usable for attacks.
//    int passHash = -1;

    QMap<QString,QString> receiveAccount; // Key: mwc713 data dir; Value: Selected account
    QMap<QString,QString> currentAccountName; // Key: mwc713 data dir; Value: Current account

    // Active window that is visible
    state::STATE activeWndState = state::STATE::LISTENING;

    // Send coins params.
    SendCoinsParams sendCoinsParams;

    // Current Path dirs
    QMap<QString,QString> pathStates;
    QMap<QString,QVector<int> > intVectorStates;

    double guiScale = -1.0;
    double initScaleValue = 1.0; // default scale value

    bool logsEnabled = true;

    bool autoStartMQSEnabled = true;
    bool autoStartTorEnabled = true;

    // Because of Custom node logic, we have to track config changes
    QMap<QString, wallet::MwcNodeConnection> nodeConnection;

    // Contact list
    QVector<ContactRecord> contactList;

    QVector<QString> walletInstancePaths; // We never remove from this list. We want to support mount/unmount paths
    int currentWalletInstanceIdx = -1;

    bool isOnlineNodeMainNetwork = true;

    bool showOutputAll = false; // Show all or Unspent outputs

    QMap<QString, qulonglong> hodlRegistrations;

    //WalletNotes* walletNotes;
    // The app context is created early on in main.cpp and tries to load the app data
    // but the wallet notes object cannot be created until the StateContext has
    // been created later in main.cpp. So we store the output and transaction notes
    // until the wallet notes object gets created at which time they are loaded into
    // the wallet notes object. Then each time the wallet notes are updated, they
    // are loaded back into these app context note maps so we don't have to
    // worry about the wallet notes object getting destroyed before the app context.
//    QMap<QString, QMap<QString, QMap<QString, QString>>> outputNotesMap;
//    QMap<QString, QMap<QString, QMap<QString, QString>>> txnNotesMap;

    // Outputs can be locked from spending.
    bool lockOutputEnabled = false; // By default it is false
    // Outputs that was locked (it is manual operation)
    // Key: output commit.
    // Value: id.  Empty string is for permanent manual ops.
    //              Non empty ID - are temporary
    QHash<QString, QString> lockedOutputs;

    // Allow users to by-pass the stem-phase of the dandelion protocol
    // and directly fluff their transactions.
    bool fluffTransactions = false;     // default to normal dandelion protocol

    // Notes data. Notes can be done for Commits, transactions or may be something else.
    // By definition tx uuid and commits are unique, that it why we can use them as a key across all wallets
    // Notes are stored in it's own location, because of data importance and corruption possibility.
    // For notes we need to do save and move
    bool notesLoaded = false;
    QMap<QString, QString> notes;

    // Earlier versions of Qt wallet stored notes in a different format by wallet and account
    // We read these notes in and migrate them to the new format for storing notes
    QMap<QString, QMap<QString, QMap<QString, QString>>> oldFormatOutputNotes;
    QMap<QString, QMap<QString, QMap<QString, QString>>> oldFormatTxnNotes;

    // Genrate proof for all send transactions. By default it is false because
    // receiver wallet need to be upgraded in order to support it.
    bool generateProof = false;

    // The display of status messages using notification windows in the lower
    // right hand corner of the wallet is enabled by default
    bool notificationWindowsEnabled = true;

    // Selected tab at the swap list page
    int swapTabSelection = 0;

    // Enforce a backup for the swaps. Before lock stage user will be asked to save the trade file somewhere
    //bool swapEnforceBackup = true;

    // Backup dir for swap trades.
    QString swapBackupDir;

    // Last used currency for swaps
    QString lastUsedSwapCurrency;

    // Backup status for the swaps
    QMap<QString, int> swapTradesBackupStatus;
    QMap<QString, int> swapMaxBackupStatus;

    // Accepted trades (we don't want ask to acceptance twice. The workflow can return back)
    QMap<QString, bool> acceptedSwaps;

    // Ban tor usage for embedded node
    bool noTorForEmbeddedNode = false;

    // For offline send page, values for slatepack and lock after
    bridge::SEND_SELECTED_METHOD sendMethod = bridge::SEND_SELECTED_METHOD::ONLINE_ID;
    bool sendLockOutput = true;

    double mktFeeReservedAmount = 1.0;
    QString mktFeeDepositAccount = "default";
    double mktFeeLevel = 0.001; // 0.1 %

    int mktPlaceSelectedBtn = 0;

    double mktPlaceMinFeeLevel = 0.0001;
    qint8  mktPlaceSelling = 2; // 0 - buy, 1 - sell, 2 - all
    QString mktPlaceCurrency = ""; // All
};

template <class T>
QMap<QString, T> & getCookieMap() {
    static QMap<QString, T> cookieMap;
    return cookieMap;
}

// add new key/value
template <class T>
void AppContext::pushCookie(QString key, T value) {
    getCookieMap<T>()[key] = value;
}

// remove and return
template <class T>
T AppContext::pullCookie(QString key) {
    return getCookieMap<T>().take(key);
}

// get is exist, don't clean up
template <class T>
T AppContext::getCookie(QString key) {
    return getCookieMap<T>().value(key);
}

}

#endif // APPCONTEXT_H

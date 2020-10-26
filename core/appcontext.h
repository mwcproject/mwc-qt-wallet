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
#include "../core/HodlStatus.h"
#include "../core/Config.h"
#include <QDebug>

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

    ContactRecord() = default;
    ContactRecord(const ContactRecord & other) = default;
    ContactRecord(const QString & n, const QString & a) { name=n; address=a; }

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
    QString getPathFor( QString name ) const;
    // update path state
    void updatePathFor( QString name, QString path );

    // IO for Int vectors
    QVector<int> getIntVectorFor( QString name ) const;
    void updateIntVectorFor( QString name, const QVector<int> & data );

    QString getReceiveAccount(const QString & walletDataDir) const { return receiveAccount.value(walletDataDir, "default");}
    void setReceiveAccount(const QString & walletDataDir, const QString & account) {receiveAccount.insert(walletDataDir, account);}

    QString getCurrentAccountName(const QString & walletDataDir) const {return currentAccountName.value(walletDataDir, "default");};
    void setCurrentAccountName(const QString & walletDataDir, const QString & currentAccount) {currentAccountName.insert(walletDataDir, currentAccount);}

    // AirdropRequests will handle differently
    void saveAirdropRequests( const QVector<state::AirdropRequests> & data );
    QVector<state::AirdropRequests> loadAirdropRequests() const;

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

    // ----- Auto Start Keybase --------
    bool isAutoStartKeybaseEnabled() const {return autoStartKeybaseEnabled;}
    void setAutoStartKeybaseEnabled(bool enabled);

    // ----- Auto Start Tor ------
    bool isAutoStartTorEnabled() const {return autoStartTorEnabled;}
    void setAutoStartTorEnabled(bool enabled);

    // ----- Outputs: All/Unspent
    bool isShowOutputAll() const {return showOutputAll;}
    void setShowOutputAll(bool all);

    wallet::MwcNodeConnection getNodeConnection( const QString & network );
    void updateMwcNodeConnection( const QString & network, const wallet::MwcNodeConnection & connection );

    // HODL registration time.
    int64_t getHodlRegistrationTime(const QString & hash) const;
    void    setHodlRegistrationTime(const QString & hash, int64_t time);

    // HODL outputs data
    void saveHodlOutputs( const QString & rootPubKeyHash, const QMap<QString, core::HodlOutputInfo> & hodlOutputs );
    QMap<QString, core::HodlOutputInfo> loadHodlOutputs(const QString & rootPubKeyHash );

    QString getNote(const QString& key);
    void updateNote(const QString& key, const QString& note);
    void deleteNote(const QString& key);

    // Outputs can be locked from spending.
    bool isLockOutputEnabled() const {return lockOutputEnabled;}
    bool isLockedOutputs(const QString & output) const;
    void setLockOutputEnabled(bool enabled);
    void setLockedOutput(const QString & output, bool lock);

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
    void setNotficationWindowsEnabled(bool enable);

    // Index of the open tab at the swap page.
    int getSwapTabSelection() const {return swapTabSselection;}
    void setSwapTabSelection(int sel) {swapTabSselection=sel;}

    bool getSwapEnforceBackup() const {return swapEnforceBackup;}
    void setSwapEnforceBackup(bool doBackup);

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
    bool autoStartKeybaseEnabled = true;
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
    QSet<QString> lockedOutputs; // Outputs that was locked (it is manual operation)

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
    int swapTabSselection = 0;

    // Enforce a backup for the swaps. Before lock stage user will be asked to save the trade file somewhere
    bool swapEnforceBackup = true;
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

// Copyright 2021 The MWC Developers
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

#include "appcontext.h"
#include "../util/ioutils.h"
#include "../util/Files.h"
#include "../util/address.h"
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QDir>
#include "../core/global.h"
#include <QtAlgorithms>
#include "../util/Log.h"
#include "../util/Process.h"
#include <QMessageBox>
#include <QCoreApplication>
#include "../core/WndManager.h"
#include <stdio.h>
#include <algorithm>
#include <QDebug>

namespace core {

const static QString settingsFileName("context.dat");
const static QString notesFileName("notes.dat");


void SendCoinsParams::saveData(QDataStream & out) const {
    out << int(0x348A4);
    out << inputConfirmationNumber;
    out << changeOutputs;
}

bool SendCoinsParams::loadData(QDataStream & in) {
    int id = -1;
    in >> id;
    if ( id!=0x348A4 )
        return false;

    in >> inputConfirmationNumber;
    in >> changeOutputs;
    return true;
}

//////////////////////////////////////////////////////////////////

ContactRecord::ContactRecord(const QString & n, const QString & a)
{
    name=n;
    address=a;
    pub_key = util::extractPubKeyFromAddress(address);
}


void ContactRecord::setData(QString _name,
                            QString _address)
{
    name = _name;
    address = _address;
    pub_key = util::extractPubKeyFromAddress(address);
}

void ContactRecord::saveData( QDataStream & out) const {
    out << 0x89365;
    out << name;
    out << address;
}

bool ContactRecord::loadData( QDataStream & in) {
    int id = 0;
    in >> id;
    if (id!=0x89365)
        return false;

    in >> name;
    in >> address;

    pub_key = util::extractPubKeyFromAddress(address);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//   AppContext

AppContext::AppContext() {
    loadData();
}

AppContext::~AppContext() {
    saveData();
}

// Get last path state. Default: Home dir
QString AppContext::getPathFor( QString name, bool returnEmptyForNew ) const {
    if (!pathStates.contains(name)) {
        if (returnEmptyForNew)
            return "";
        else
            return QDir::homePath();
    }

    return pathStates[name];
}

// update path state
void AppContext::updatePathFor( QString name, QString path ) {
    pathStates[name] = path;
}

QVector<int> AppContext::getIntVectorFor( QString name ) const {
    if ( !intVectorStates.contains(name) )
        return QVector<int>();

    return intVectorStates[name];
}

void AppContext::updateIntVectorFor( QString name, const QVector<int> & data ) {
    intVectorStates[name] = data;
}


bool AppContext::loadData() {
    bool res = loadDataImpl();

    if (walletInstancePaths.isEmpty()) {
        // Need to do default initialization
        // Let's scan for the wallets.
        QPair<bool,QString> path = ioutils::getAppDataPath("");
        if (path.first) {
            QDir qt_wallet_dir(path.second);
            QFileInfoList files = qt_wallet_dir.entryInfoList();
            for (const QFileInfo & fl : files) {
                if (fl.isDir()) {
                    QString walletDataDir = fl.fileName(); // The last dir name, for QT developers it is a file.
                    if (walletDataDir.startsWith('.'))
                        continue; // Self and parent - not interested
                    if (wallet::WalletConfig::doesSeedExist(walletDataDir)) {
                        QString arch = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(walletDataDir,this)[1];
                        if (arch == util::getBuildArch() ) {
                            walletInstancePaths.push_back(walletDataDir);
                            currentWalletInstanceIdx = 0;
                        }
                    }
                }
            }
        }
    }

    return res;
}

bool AppContext::loadDataImpl() {
    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        QMessageBox::critical(nullptr, "Error", dataPath.second);
        QCoreApplication::exit();
        return false;
    }

    QFile file(dataPath.second + "/" + settingsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_7);

    int id = 0;
    in >> id;

    if (id<0x4783 || id>0x47AB)
        return false;

    QString mockStr;
    in >> mockStr;
    in >> mockStr;

    int st;
    in >> st;
    activeWndState = (state::STATE)st;

    if (activeWndState == state::STATE::RESYNC || activeWndState == state::STATE::MIGRATION ) {
        // Invalid states, let's regirect to Node info
        activeWndState = state::STATE::NODE_INFO;
    }

    // for the mobile wallet we always want to start from the home page.
#ifdef WALLET_MOBILE
    activeWndState = state::STATE::WALLET_HOME;
#endif

    in >> pathStates;
    in >> intVectorStates;

    sendCoinsParams.loadData(in);

    int contSz = 0;
    in >> contSz;
    contactList.clear();
    for (int i=0;i<contSz;i++) {
        core::ContactRecord cnt;
        if (cnt.loadData(in))
            contactList.push_back(cnt);
        else
            return false;
    }

    if (id>=0x4784)
        in >> guiScale;

    if (id>=0x4785)
        in >> logsEnabled;

    wallet::MwcNodeConnectionDeprecated nodeConnectionMainNet;
    wallet::MwcNodeConnectionDeprecated nodeConnectionFlooNet;

    if (id>=0x4786) {
        nodeConnectionMainNet.loadData(in);
        nodeConnectionFlooNet.loadData(in);
    }

    if (id>=0x4787) {
        QString s;
        in >> s;
        in >> s;
    }

    if (id>=0x4788) {
        in >> showOutputAll;
    }

    if (id>=0x4789) {
        QMap<QString, qulonglong> hodlRegistrations;
        in >> hodlRegistrations;
    }

    if (id>=0x4790) {
        bool b;
        in >> b;
    }

    if (id>=0x4791) {
        bool b; // legacy autoStartKeybaseEnabled
        in >> b;
    }

    // the format for saving notes and where they are saved has changed
    // read in any old notes so they can be migrated when notes are accessed
    if (id>=0x4792) {
        in >> oldFormatOutputNotes;
    }

    if (id>=0x4793) {
        in >> oldFormatTxnNotes;
    }

    if (id>=0x4794) {
        in >> lockOutputEnabled;
        QSet<QString> outs;
        in >> outs;
        // Reading only permanent locks without id
        // Load expected to call only, so we can clean safely
        lockedOutputs.clear();
        for (const auto & o : outs)
            lockedOutputs.insert(o, "");
    }

    if (id>=0x4795) {
        in >> fluffTransactions;
    }

    if (id>=0x4796) {
        in >> receiveAccount;
        in >> currentAccountName;
    }

    if (id>=0x4797) {
        bool b;
        in >> b;
    }

    if (id>=0x4798) {
        int sz = 0;
        in >> sz;
        for (int r=0; r<sz; r++) {
            QString key;
            in >> key;
            wallet::MwcNodeConnectionDeprecated val;
            val.loadData(in);
        }

        in >> walletInstancePaths;
        in >> currentWalletInstanceIdx;
    }

    if (id>=0x4799) {
        in >> isOnlineNodeMainNetwork;
    }

    if (id>=0x479A) {
        in >> generateProof;
    }

    if (id>=0x479B) {
        in >> notificationWindowsEnabled;
    }

#ifdef Q_OS_WIN
    // Disable in windows because notification bring the whole QT wallet on the top of other windows.
    // Notications overlap other windows.
    notificationWindowsEnabled = false;
#endif

    if (id>=0x479C) {
        in >> swapTabSelection;
    }

    if (id>=0x479D) {
        bool b = true;
        in >> b;
    }

    if (id>=0x479E) {
        in >> lastUsedSwapCurrency;
    }

    if (id>=0x479F) {
        in >> swapTradesBackupStatus;
    }

    if (id>=0x47A0) {
        in >> swapMaxBackupStatus;
    }

    if (id>=0x47A1) {
        in >> acceptedSwaps;
    }

    if (id>=0x47A2) {
        bool noTorForEmbeddedNode;
        in >> noTorForEmbeddedNode;
    }

    if (id>=0x47A3) {
        int sm = bridge::SEND_SELECTED_METHOD::ONLINE_ID;
        in >> sm;
        sendMethod = bridge::SEND_SELECTED_METHOD(sm);
        in >> sendLockOutput;
    }

    if (id>=0x47A4) {
        in >> mktFeeReservedAmount;
        in >> mktFeeDepositAccount;
        in >> mktFeeLevel;

        in >> mktPlaceSelectedBtn;
        double d;
        in >> d;// mktPlaceMinFeeLevel;
        qint8 i8;
        in >> i8;//mktPlaceSelling;
        QString s;
        in >> s; //mktPlaceCurrency;
        in >> d;
        in >> d;
    }

    if (id>=0x47A5) {
        in >> swapBackupDir;
    }

    if (id>=0x47A6) {
        QString tmp;
        in >> tmp;
        in >> tmp;
    }

    if (id>=0x47A7) {
        in >> transactionsWithCongrats;
    }

    if (id>=0x47A8) {
        in >> sendSlatepacks;
        in >> receiveSlatepacks;
    }

    if (id>=0x47A9) {
        in >> featureSlatepack;
        in >> featureMWCMQS;
        in >> featureTor;
    }

    if (id>=0x47AA) {
        in >> faucetRequest;
    }

    if (id >= 0x47AB) {
        in >> walletParams;
    }

    return true;
}


void AppContext::saveData() const {
    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        core::getWndManager()->messageTextDlg("Error", dataPath.second);
        QCoreApplication::exit();
        return;
    }

    QString filePath = dataPath.second + "/" + settingsFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        core::getWndManager()->messageTextDlg(
              "ERROR",
              "Unable to save gui-wallet settings to " + filePath +
              "\nError: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);

    QString mockStr;

    out << 0x47AB;
    out << mockStr;
    out << mockStr;
    out << int(activeWndState);
    out << pathStates;
    out << intVectorStates;

    sendCoinsParams.saveData(out);

    out << (int)contactList.size();
    for ( const auto & c : contactList ) {
        c.saveData(out);
    }

    out << guiScale;
    out << logsEnabled;

    wallet::MwcNodeConnectionDeprecated nc;
    nc.saveData(out);
    nc.saveData(out);

    QString s;
    out << s;
    out << s;

    out << showOutputAll;

    QMap<QString, qulonglong> hodlRegistrations;
    out << hodlRegistrations;

    out << true;
    bool b = false; // legacy autoStartKeybaseEnabled
    out << b;

    // if dialogs which display notes have not been accessed, the
    // older format notes will not have been migrated, so we need to
    // continue to save them
    out << oldFormatOutputNotes;
    out << oldFormatTxnNotes;

    out << lockOutputEnabled;

    // Storing only a set of keys with empty values
    QSet<QString> lockedOuts;
    for ( auto io = lockedOutputs.begin(); io != lockedOutputs.end(); io++ ) {
        if (io.value().isEmpty())
            lockedOuts += io.key();
    }
    out << lockedOuts;

    out << fluffTransactions;

    out << receiveAccount;
    out << currentAccountName;

    out << true;

    int sz = 0;// nodeConnection.size();
    out << sz;
/*    for (QMap<QString, wallet::MwcNodeConnection>::const_iterator i = nodeConnection.constBegin(); i != nodeConnection.constEnd(); ++i) {
        out << i.key();
        i.value().saveData(out);
    }*/
    out << walletInstancePaths;
    out << currentWalletInstanceIdx;

    out << isOnlineNodeMainNetwork;

    out << generateProof;

    out << notificationWindowsEnabled;

    out << swapTabSelection;

    b = true; // legacy swapEnforceBackup
    out << b;

    out << lastUsedSwapCurrency;

    out << swapTradesBackupStatus;

    out << swapMaxBackupStatus;

    out << acceptedSwaps;

    bool noTorForEmbeddedNode = false;
    out << noTorForEmbeddedNode;

    int sm = int(sendMethod);
    out << sm;
    out << sendLockOutput;

    out << mktFeeReservedAmount;
    out << mktFeeDepositAccount;
    out << mktFeeLevel;
    out << mktPlaceSelectedBtn;
    // Feedback,
    double d = 0.0;
    out << d; // mktPlaceMinFeeLevel;
    qint8 i8 = 0;
    out << i8; // mktPlaceSelling;
    out << s; // mktPlaceCurrency;
    out << d;
    out << d;

    out << swapBackupDir;

    out << QString(""); //torBridgeLine;
    out << QString(""); //torClientOption;

    out << transactionsWithCongrats;

    out << sendSlatepacks;
    out << receiveSlatepacks;

    out << featureSlatepack;
    out << featureMWCMQS;
    out << featureTor;
    out << faucetRequest;

    out << walletParams;
}

void AppContext::loadNotesData() {
    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        QMessageBox::critical(nullptr, "Error", dataPath.second);
        QCoreApplication::exit();
        return;
    }

    notesLoaded = true;

    QFile file(dataPath.second + "/" + notesFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exists with notes in new format
        // migrate any notes in the old format to the new format
        migrateOutputNotes();
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_7);

    int id = 0;
    in >> id;

    if (id!=0x4580) {
        // migrate any notes in the old format to the new format
        migrateOutputNotes();
        return;
    }

    in >> notes;
    // migrate any notes in the old format to the new format
    // the old format notes will be added to the notes map
    migrateOutputNotes();
}

void AppContext::saveNotesData() const {
    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        core::getWndManager()->messageTextDlg("Error", dataPath.second);
        QCoreApplication::exit();
        return;
    }

    QString filePath = dataPath.second + "/" + notesFileName;
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            core::getWndManager()->messageTextDlg(
                    "ERROR",
                    "Unable to save Notes data to " + filePath +
                    "\nError: " + file.errorString());
            return;
        }

        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_7);

        out << 0x4580;
        out << notes;

        if (out.status() != QDataStream::Ok) {
            core::getWndManager()->messageTextDlg(
                    "ERROR",
                    "Unable to save Notes data to " + filePath);
            return;
        }
        file.close();
    }
}

void AppContext::migrateOutputNotes()
{
    if (oldFormatOutputNotes.size() <= 0)
        return;

    // if any older format output notes have been read in, convert them to
    // the new format where they are stored without wallet and account info
    for (QString walletId : oldFormatOutputNotes.keys()) {
        QMap<QString, QMap<QString, QString>>& accountNotes = oldFormatOutputNotes[walletId];
        for (QString accountId : accountNotes.keys())
        {
            QMap<QString, QString>& op_notes = accountNotes[accountId];
            for (QString commitment : op_notes.keys()) {
                QString note = op_notes.value(commitment);
                QString key = "c_" + commitment;
                notes.insert(key, note);
            }
        }
    }
    saveNotesData();
    oldFormatOutputNotes.clear();
}

bool AppContext::hasTxnNotesToMigrate() {
    return oldFormatTxnNotes.size() > 0;
}

QStringList AppContext::getTxnNotesToMigrate(QString walletId, QString accountId) {
    QStringList txIdxList;

    if (oldFormatTxnNotes.size() <= 0)
        return txIdxList;

    if (oldFormatTxnNotes.contains(walletId)) {
        QMap<QString, QMap<QString, QString>> accountNotes = oldFormatTxnNotes[walletId];
        if (accountNotes.contains(accountId)) {
           QMap<QString, QString> txnNotes = accountNotes[accountId];
           for (QString txIdx : txnNotes.keys()) {
               txIdxList.append(txIdx);
           }
        }
    }
    return txIdxList;
}

void AppContext::migrateTxnNote(QString walletId, QString accountId, QString txIdx, QString txUuid) {
    if (oldFormatTxnNotes.size() <= 0)
        return;

    if (oldFormatTxnNotes.contains(walletId)) {
        QMap<QString, QMap<QString, QString>>& accountNotes = oldFormatTxnNotes[walletId];
        if (accountNotes.contains(accountId)) {
            QMap<QString, QString>& txnNotes = accountNotes[accountId];
            if (txnNotes.contains(txIdx)) {
                QString note = txnNotes.value(txIdx);
                QString key = "tx_" + txUuid;
                updateNote(key, note);
                txnNotes.remove(txIdx);
            }
            if (txnNotes.size() <= 0) {
                // this account doesn't have any more notes, so remove it
                accountNotes.remove(accountId);
            }
        }
        if (accountNotes.size() <= 0) {
            // this wallet doesn't have any more notes
            oldFormatTxnNotes.remove(walletId);
        }
    }
}


void AppContext::setLogsEnabled(bool enabled) {
    if (enabled == logsEnabled)
        return;
    logsEnabled = enabled;
    saveData();
}

//bool AppContext::useTorForNode() const {
/*    bool tor = isAutoStartTorEnabled();
    if (config::isOnlineNode())
        tor = true;
    return tor && (!noTorForEmbeddedNode);*/
//    return !noTorForEmbeddedNode;
//}

void AppContext::setShowOutputAll(bool all) {
    showOutputAll = all;
}

/*void AppContext::setNoTorForEmbeddedNode(bool noTor) {
    if (noTorForEmbeddedNode == noTor)
        return;

    noTorForEmbeddedNode = noTor;
    saveData();
}*/

void AppContext::setSendMethod(bridge::SEND_SELECTED_METHOD _sendMethod) {
    sendMethod = _sendMethod;
}

void AppContext::setSendLockOutput(bool lock)  {
    sendLockOutput = lock;
}

// -------------- Contacts

// Add new contact
QPair<bool, QString> AppContext::addContact( const ContactRecord & contact ) {
    // check for duplication...
    // Check for names duplication...
    for ( const auto & cont : contactList ) {
        if (cont.name == contact.name)
            return QPair<bool, QString>(false, "Contact '" + contact.name + "' already exist.");
    }

    contactList.push_back(contact);
    std::sort(contactList.begin(), contactList.end(), [](const ContactRecord &c1, const ContactRecord &c2) { return c1.name < c2.name; } );
    saveData();
    return QPair<bool, QString>(true, "");
}

// Remove contact. return false if not found
QPair<bool, QString> AppContext::deleteContact( const ContactRecord & contact ) {

    for ( int i=0; i<contactList.size(); i++ ) {
        if ( contactList[i] == contact ) {
            contactList.remove(i);
            saveData();
            return QPair<bool, QString>(true, "");
        }
    }
    return QPair<bool, QString>(false, "Contact '" + contact.name + "' not found. Unable to delete it.");
}

// Update contact
QPair<bool, QString> AppContext::updateContact( const ContactRecord & prevValue, const ContactRecord & newValue ) {
    for ( int i=0; i<contactList.size(); i++ ) {
        if ( contactList[i] == prevValue ) {
            contactList[i] = newValue;
            std::sort(contactList.begin(), contactList.end(), [](const ContactRecord &c1, const ContactRecord &c2) { return c1.name < c2.name; } );
            saveData();
            return QPair<bool, QString>(true, "");
        }
    }
    return QPair<bool, QString>(false, "Contact '" + prevValue.name + "' not found. Unable to update it.");
}

double AppContext::getGuiScale() const
{
    return guiScale<0.0 ? initScaleValue : guiScale;
}

QPair<QString, QString> AppContext::getOnlineNodeWalletNetworkAndPath() const {
    QString network = isOnlineNodeRunsMainNetwork() ? "Mainnet" : "Floonet";
    QString walletPath = QString("tmp") + QDir::separator() + "online_node_wallet"  + QDir::separator() + network;
    return QPair<QString, QString>(network, walletPath);
}

// Wallet instances. Return instances paths that are valid and current selected index
QPair<QVector<QString>, int> AppContext::getWalletInstances(bool hasSeed) const {
    switch (config::getWalletRunMode()) {
        case config::WALLET_RUN_MODE::COLD_WALLET:
        case config::WALLET_RUN_MODE::ONLINE_WALLET: {
            QVector<QString> paths(walletInstancePaths);

            if (hasSeed) {
                for ( int r=paths.size()-1; r>=0; r-- ) {
                    if (!wallet::WalletConfig::doesSeedExist(paths[r])) {
                        paths.remove(r);
                    }
                }
            }

            int selectedIdx = 0;
            if (!paths.isEmpty()) {
                selectedIdx = paths.indexOf(walletInstancePaths.value(currentWalletInstanceIdx, ""));
                if (selectedIdx<0)
                    selectedIdx = 0;
            }
            // On very forst run paths.size() is 0
            Q_ASSERT(selectedIdx==0 || (selectedIdx>=0 && selectedIdx<paths.size()));

            return QPair<QVector<QString>, int> (paths, selectedIdx);
        }
        case config::WALLET_RUN_MODE::ONLINE_NODE: {
            // For network the wallet instance is predefined.
            QPair<QString, QString> nw_path = getOnlineNodeWalletNetworkAndPath();
            QVector<QString> paths{ nw_path.second };
            return QPair<QVector<QString>, int> (paths,0);
        }
    }
    Q_ASSERT(false);
    return QPair<QVector<QString>, int> ({},0);
}
// Return path to current wallet instance. Expected that at least 1 instance does exist
QString AppContext::getCurrentWalletInstance(bool hasSeed) const {
    auto res = getWalletInstances(hasSeed);
    Q_ASSERT(!res.first.isEmpty());
    if (res.first.isEmpty())
      return "";

    return res.first[res.second];
}

void AppContext::setCurrentWalletInstance(const QString & path) {
    int idx = walletInstancePaths.indexOf(path);
    if (idx>=0) {
        currentWalletInstanceIdx = idx;
    }
}


// Add new instance and make it active
void AppContext::addNewInstance(const QString & instance) {
    currentWalletInstanceIdx = walletInstancePaths.indexOf(instance);
    if (currentWalletInstanceIdx < 0) {
        currentWalletInstanceIdx = walletInstancePaths.size();
        walletInstancePaths.push_back(instance);
    }
    saveData();
}

// Check if inline node running the main network
bool AppContext::isOnlineNodeRunsMainNetwork() const {
    return isOnlineNodeMainNetwork;
}

void AppContext::setOnlineNodeRunsMainNetwork(bool isMainNet) {
    if (isOnlineNodeMainNetwork==isMainNet)
        return;
    isOnlineNodeMainNetwork = isMainNet;
    saveData();
}

// Generate proof for all send transactions.
void AppContext::setGenerateProof(bool proof) {
    generateProof = proof;
    // flag not critical, save on exit
}


static QString imp_val("IMPOSSIBLE_VALUE");

QPair<bool, QString> AppContext::isLockedOutputs(const QString & output) const {
    if (!lockedOutputs.contains(output))
        return QPair<bool, QString>(false, "");

    QString val = lockedOutputs.value(output, imp_val);

    if (val==imp_val)
        return QPair<bool, QString>(false, "");

    if (!lockOutputEnabled && val.isEmpty()) // manual locking is off, let's skip it
        return QPair<bool, QString>(false, "");

    return QPair<bool, QString>(true, val);
}

void AppContext::setLockOutputEnabled(bool enabled, wallet::Wallet * wallet) {
    if (lockOutputEnabled == enabled)
        return;

    lockOutputEnabled = enabled;
    saveData();
    wallet->emitWalletBalanceUpdated();
}

void AppContext::setLockedOutput(const QString & output, bool lock, QString id, wallet::Wallet * wallet) {
    if (lock) {
            lockedOutputs.insert(output, id);
            saveData();
            wallet->emitWalletBalanceUpdated();
    }
    else {
        QString prevKey = lockedOutputs.value(output, "");
        if (prevKey != id) {
            core::getWndManager()->messageTextDlg("Warning", "You can't unlock this output, it is reserved by one of wallet running operation.");
            return;
        }
        if (lockedOutputs.remove(output)) {
            saveData();
            wallet->emitWalletBalanceUpdated();
        }
    }
}

void AppContext::unlockOutputsById(QString id) {
    QMutableHashIterator<QString, QString> i(lockedOutputs);
    QVector<QString> updatedOutputs;
    while (i.hasNext()) {
        i.next();
        if (i.value()==id) {
            updatedOutputs += i.key();
            i.remove();
        }
    }

    if (!updatedOutputs.isEmpty()) {
        saveData();
    }
}

QVector<QString> AppContext::getLockedOutputsById(QString id) const {
    QVector<QString> result;

    for ( auto i=lockedOutputs.begin(); i!=lockedOutputs.end(); i++ ) {
        if (i.value() == id) {
            result.push_back(i.key());
        }
    }

    return result;
}

QStringList AppContext::getLockedOutputs() const {
    QStringList result;
    if (!lockOutputEnabled)
        return result;

    for ( auto i=lockedOutputs.begin(); i!=lockedOutputs.end(); i++ ) {
        result.append(i.key());
    }
    return result;
}


void AppContext::setFluff(bool fluffSetting) {
    if (fluffTransactions == fluffSetting)
        return;
    fluffTransactions = fluffSetting;
    saveData();
}

QString AppContext::getNote(const QString& key) {
    if (!notesLoaded) {
        loadNotesData();
    }
    return notes.value(key);
}

void AppContext::updateNote(const QString& key, const QString& note) {
    if (!notesLoaded) {
        loadNotesData();
    }

    if (note.isEmpty())
        notes.remove(key);
    else
        notes.insert(key, note);
    saveNotesData();
}

void AppContext::deleteNote(const QString& key) {
    if (!notesLoaded) {
        loadNotesData();
    }
    notes.remove(key);
    saveNotesData();
}

void AppContext::setNotificationWindowsEnabled(bool enable) {
    if (notificationWindowsEnabled == enable)
        return;
    notificationWindowsEnabled = enable;
    saveData();
}

void AppContext::setSwapBackupDir(QString backupDir) {
    if (swapBackupDir == backupDir)
        return;

    swapBackupDir = backupDir;
    saveData();
}

int  AppContext::getSwapBackStatus(const QString & swapId) const {
    return swapTradesBackupStatus.value(swapId, 0);
}

void AppContext::setSwapBackStatus(const QString & swapId, int status) {
    swapTradesBackupStatus[swapId] = status;
    saveData();
}

int AppContext::getMaxBackupStatus(QString swapId, int status) {
    int st = swapMaxBackupStatus.value(swapId, 0);
    st = std::max(st,status);
    swapMaxBackupStatus.insert( swapId, st );
    return st;
}

// Check fir accepted trades (we don't want ask to acceptance twice. The workflow can return back)
bool AppContext::isTradeAccepted(const QString & swapId) const {
    return acceptedSwaps.value(swapId, false);
}

void AppContext::setTradeAcceptedFlag(const QString & swapId, bool accepted) {
    acceptedSwaps.insert(swapId, accepted);
}

void AppContext::setMktFeeLevel(const double &feeLevel) {
    mktFeeLevel = feeLevel;
}

void AppContext::setMktFeeDepositAccount(QString accountName) {
    mktFeeDepositAccount = accountName;
}

void AppContext::setMktFeeReserveAmount(const double &reservedAmount) {
    mktFeeReservedAmount = reservedAmount;
}

void AppContext::setMktPlaceSelectedBtn(int btn) {
    mktPlaceSelectedBtn = btn;
}

void AppContext::setSwapMktMinFeeLevel(double fee) {
    mktPlaceMinFeeLevel = fee;
}
void AppContext::setSwapMktSelling(int value) {
    mktPlaceSelling = value;
}
void AppContext::setSwapMktCurrency(QString currency) {
    mktPlaceCurrency = currency;
}

void AppContext::setShowCongratsForTx(const QString & txUUID) {
    transactionsWithCongrats.insert(txUUID);
    saveData();
}

void AppContext::addSendSlatepack(const QString & txUUID, QString slatepack) {
    if (sendSlatepacks.contains(txUUID) && sendSlatepacks[txUUID]==slatepack)
        return;
    sendSlatepacks[txUUID]=slatepack;
    saveData();
}

void AppContext::addReceiveSlatepack(const QString & txUUID, QString slatepack) {
    if (receiveSlatepacks.contains(txUUID) && receiveSlatepacks[txUUID]==slatepack)
        return;
    receiveSlatepacks[txUUID]=slatepack;
    saveData();
}

void AppContext::deleteSendSlatepack(const QString & txUUID) {
    if (!sendSlatepacks.contains(txUUID))
        return;
    sendSlatepacks.remove(txUUID);
    saveData();
}
void AppContext::deleteReceiveSlatepack(const QString & txUUID) {
    if (!receiveSlatepacks.contains(txUUID))
        return;
    receiveSlatepacks.remove(txUUID);
    saveData();
}

bool AppContext::isFeatureSlatepack() const {
    if (config::isOnlineWallet())
        return featureSlatepack;
    else
        return true;
}
bool AppContext::isFeatureMWCMQS() const {
    if (config::isOnlineWallet())
        return featureMWCMQS;
    else
        return false;
}
bool AppContext::isFeatureTor() const {
    if (config::isOnlineWallet())
        return featureTor;
    else
        return false;
}


void AppContext::setFeatureSlatepack(bool val) {
    if (!config::isOnlineWallet())
        return;

    if (featureSlatepack==val)
        return;
    featureSlatepack = val;
    saveData();
}
void AppContext::setFeatureMWCMQS(bool val) {
    if (!config::isOnlineWallet())
        return;

    if (featureMWCMQS==val)
        return;
    featureMWCMQS = val;
    saveData();
}
void AppContext::setFeatureTor(bool val) {
    if (!config::isOnlineWallet())
        return;

    if (featureTor==val)
        return;
    featureTor = val;
    saveData();
}

bool AppContext::isFaucetRequested() {
    return faucetRequest;
}

void AppContext::faucetRequested() {
    if (faucetRequest)
        return;

    faucetRequest = true;
    saveData();
}

QString AppContext::getWalletParam(int walletId, QString key, QString defaultValue) {
    QString dtKey = QString::number(walletId) + "_" + key;
    return walletParams.value(dtKey, defaultValue);
}

void AppContext::setWalletParam(int walletId, QString key, QString value, bool saveDataNow) {
    QString dtKey = QString::number(walletId) + "_" + key;
    walletParams.insert(dtKey, value);
    if (saveDataNow) {
        saveData();
    }
}


}

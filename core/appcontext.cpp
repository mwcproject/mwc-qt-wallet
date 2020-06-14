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

#include "appcontext.h"
#include "../util/ioutils.h"
#include "../util/Files.h"
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QDir>
#include "../core/global.h"
#include <QtAlgorithms>
#include "../util/Log.h"
#include <QMessageBox>
#include <QCoreApplication>
#include "../core/WndManager.h"
#include <stdio.h>

namespace core {

const static QString settingsFileName("context.dat");
const static QString notesFileName("notes.dat");
const static QString airdropRequestsFileName("requests.dat");
const static QString hodlOutputsPrefix("ho_");


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

void ContactRecord::setData(QString _name,
                            QString _address)
{
    name = _name;
    address = _address;
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
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//   AppContext

AppContext::AppContext() {
    loadData();

    // Check if airdrop request need to be cleaned up
    QVector<state::AirdropRequests> airDropData = loadAirdropRequests();

    int64_t cleanTimeLimit = QDateTime::currentMSecsSinceEpoch() - mwc::AIRDROP_TRANS_KEEP_TIME_MS;
    int sz = airDropData.size();
    for (int t=sz-1; t>=0; t--) {
        if (airDropData[t].timestamp < cleanTimeLimit) {
            airDropData.remove(t);
        }
    }

    if (sz != airDropData.size())
        saveAirdropRequests(airDropData);
}

AppContext::~AppContext() {
    saveData();
}

// Get last path state. Default: Home dir
QString AppContext::getPathFor( QString name ) const {
    if (!pathStates.contains(name))
        return QDir::homePath();

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

    if (id<0x4783 || id>0x4797)
         return false;

    QString mockStr;
    in >> mockStr;
    in >> mockStr;

    int st;
    in >> st;
    activeWndState = (state::STATE)st;

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

    if (id>=0x4786) {
        nodeConnectionMainNet.loadData(in);
        nodeConnectionFlooNet.loadData(in);
    }

    if (id>=0x4787) {
        in >> wallet713DataPath;
        in >> network;
    }

    if (id>=0x4788) {
        in >> showOutputAll;
    }

    if (id>=0x4789) {
        in >> hodlRegistrations;
    }

    if (id>=0x4790) {
        in >> autoStartMQSEnabled;
    }

    if (id>=0x4791) {
        in >> autoStartKeybaseEnabled;
    }

    QMap<QString, QMap<QString, QMap<QString, QString>>> mock;
    if (id>=0x4792) {
        in >> mock; //outputNotesMap;
    }

    if (id>=0x4793) {
        in >> mock; //txnNotesMap;
    }

    if (id>=0x4794) {
        in >> lockOutputEnabled;
        in >> lockedOutputs;
    }

    if (id>=0x4795) {
        in >> fluffTransactions;
    }

    if (id>=0x4796) {
        in >> receiveAccount;
        in >> currentAccountName;
    }

    if (id>=0x4797)
        in >> autoStartTorEnabled;

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

    out << 0x4797;
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

    nodeConnectionMainNet.saveData(out);
    nodeConnectionFlooNet.saveData(out);

    out << wallet713DataPath;
    out << network;

    out << showOutputAll;

    out << hodlRegistrations;

    out << autoStartMQSEnabled;
    out << autoStartKeybaseEnabled;

    QMap<QString, QMap<QString, QMap<QString, QString>>> mock;
    out << mock; //outputNotesMap;
    out << mock; //txnNotesMap;

    out << lockOutputEnabled;
    out << lockedOutputs;

    out << fluffTransactions;

    out << receiveAccount;
    out << currentAccountName;

    out << autoStartTorEnabled;
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
        // first run, no file exist
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_7);

    int id = 0;
    in >> id;

    if (id!=0x4580)
        return;

    in >> notes;
}

void AppContext::saveNotesData() const {
    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        core::getWndManager()->messageTextDlg("Error", dataPath.second);
        QCoreApplication::exit();
        return;
    }

    QString filePath = dataPath.second + "/" + notesFileName + ".bak";
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
    // Rename suppose to be atomic, no data loss expected
    int res = std::rename( filePath.toStdString().c_str(), (dataPath.second + "/" + notesFileName).toStdString().c_str() );
    if (res!=0) {
        core::getWndManager()->messageTextDlg(
                "ERROR",
                "Unable to save Notes data, file move system error code: " + QString::number(res));
    }
}


void AppContext::setLogsEnabled(bool enabled) {
    if (enabled == logsEnabled)
        return;
    logsEnabled = enabled;
    saveData();
}

void AppContext::setAutoStartMQSEnabled(bool enabled) {
    if (enabled == autoStartMQSEnabled)
        return;
    autoStartMQSEnabled = enabled;
    saveData();
}

void AppContext::setAutoStartTorEnabled(bool enabled) {
    if (enabled == autoStartTorEnabled)
        return;
    autoStartTorEnabled = enabled;
    saveData();
}

void AppContext::setAutoStartKeybaseEnabled(bool enabled) {
    if (enabled == autoStartKeybaseEnabled)
        return;
    autoStartKeybaseEnabled = enabled;
    saveData();
}

void AppContext::setShowOutputAll(bool all) {
    showOutputAll = all;
}


// AirdropRequests will handle differently
void AppContext::saveAirdropRequests( const QVector<state::AirdropRequests> & data ) {
    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        core::getWndManager()->messageTextDlg("Error", dataPath.second);
        QCoreApplication::exit();
        return;
    }

    QString filePath = dataPath.second + "/" + airdropRequestsFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        core::getWndManager()->messageTextDlg("ERROR",
                                     "Unable save to aidrop requests to " + filePath +
                                     "\nError: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);

    out << 0x8327d1;
    int sz = data.size();
    out << sz;
    for (auto & d : data ) {
        d.saveData(out);
    }
}

QVector<state::AirdropRequests> AppContext::loadAirdropRequests() const {

    QVector<state::AirdropRequests> res;

    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        core::getWndManager()->messageTextDlg("Error", dataPath.second);
        QCoreApplication::exit();
        return res;
    }

    QFile file(dataPath.second + "/" + airdropRequestsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return res;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_7);

    int id = 0;
    in >> id;
    if (id!=0x8327d1)
        return res;

    int sz = 0;
    in >> sz;

    while(sz>0) {
        sz--;
        state::AirdropRequests req;
        if (!req.loadData(in))
            break;
        res.push_back(req);
    }

    return res;
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

wallet::MwcNodeConnection AppContext::getNodeConnection(const QString network) const {
    return network.toLower().contains("main") ? nodeConnectionMainNet : nodeConnectionFlooNet;
}

void AppContext::updateMwcNodeConnection(const QString network, const wallet::MwcNodeConnection & connection ) {
    if (network.toLower().contains("main")) {
        nodeConnectionMainNet = connection;
        if (nodeConnectionFlooNet.notCustom() && connection.notCustom() )
            nodeConnectionFlooNet = connection;
    }
    else {
        nodeConnectionFlooNet = connection;
        if (nodeConnectionMainNet.notCustom() && connection.notCustom() )
            nodeConnectionMainNet = connection;

    }
    // save because it must be in sync with configs that saved now as well
    saveData();
}

bool AppContext::getWallet713DataPathWithNetwork( QString & _wallet713DataPath, QString & _network) {
    if ( wallet713DataPath.isEmpty() || network.isEmpty() )
        return false;

    _wallet713DataPath = wallet713DataPath;
    _network = network;
    return true;
}

void AppContext::setWallet713DataPathWithNetwork( const QString & _wallet713DataPath, const QString & _network ) {
    if ( wallet713DataPath == _wallet713DataPath && network == _network )
        return;

    wallet713DataPath = _wallet713DataPath;
    network = _network;
    saveData();
}

// HODL registration time.
int64_t AppContext::getHodlRegistrationTime(const QString & hash) const {
    return hodlRegistrations.value(hash, 0);
}

void AppContext::setHodlRegistrationTime(const QString & hash, int64_t time) {
    hodlRegistrations.insert( hash, qlonglong(time) );
}

void AppContext::saveHodlOutputs( const QString & rootPubKeyHash, const QMap<QString, core::HodlOutputInfo> & hodlOutputs ) {
    Q_ASSERT(!rootPubKeyHash.isEmpty());
    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        core::getWndManager()->messageTextDlg("Error", dataPath.second);
        QCoreApplication::exit();
        return;
    }

    QString filePath = dataPath.second + "/" + hodlOutputsPrefix+rootPubKeyHash + ".dat";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        core::getWndManager()->messageTextDlg("ERROR",
                                         "Unable to store HODL cache to " + filePath +
                                         "\nError: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);

    out << 0x324745;
    int sz = hodlOutputs.size();
    out << sz;
    for (auto ho = hodlOutputs.begin(); ho!=hodlOutputs.end(); ho++ ) {
        ho.value().saveData(out);
    }

}
QMap<QString, core::HodlOutputInfo> AppContext::loadHodlOutputs(const QString & rootPubKeyHash ) {
    QMap<QString, core::HodlOutputInfo> res;

    QPair<bool,QString> dataPath = ioutils::getAppDataPath("context");
    if (!dataPath.first) {
        core::getWndManager()->messageTextDlg("Error", dataPath.second);
        QCoreApplication::exit();
        return res;
    }

    QString filePath = dataPath.second + "/" + hodlOutputsPrefix+rootPubKeyHash + ".dat";

    QFile file(filePath);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return res;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_7);

    int id = 0;
    in >> id;
    if (id!=0x324745)
        return res;

    int sz = 0;
    in >> sz;

    while(sz>0) {
        sz--;
        core::HodlOutputInfo req;
        if (!req.loadData(in))
            break;
        res.insert( req.outputCommitment, req );
    }

    return res;

}

bool AppContext::isLockedOutputs(const QString & output) const {
    if (!lockOutputEnabled)
        return false;

    return lockedOutputs.contains(output);
}

void AppContext::setLockOutputEnabled(bool enabled) {
    if (lockOutputEnabled == enabled)
        return;

    lockOutputEnabled = enabled;
    saveData();
}

void AppContext::setLockedOutput(const QString & output, bool lock) {
    if (lock) {
        if ( !lockedOutputs.contains(output) ) {
            lockedOutputs.insert(output);
            saveData();
            logger::logEmit("AppContext", "onOutputLockChanged", output + " locked" );
            emit onOutputLockChanged(output);
        }
    }
    else {
        if (lockedOutputs.remove(output)) {
            saveData();
            logger::logEmit("AppContext", "onOutputLockChanged", output + " unlocked" );
            emit onOutputLockChanged(output);
        }
    }
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
    Q_ASSERT(notesLoaded); // we suppose to load first
    if (note.isEmpty())
        notes.remove(key);
    else
        notes.insert(key, note);
    saveNotesData();
}

void AppContext::deleteNote(const QString& key) {
    Q_ASSERT(notesLoaded); // we suppose to load first
    notes.remove(key);
    saveNotesData();
}

}

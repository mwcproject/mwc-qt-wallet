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
#include "../control/messagebox.h"
#include "../core/global.h"
#include <QtAlgorithms>

namespace core {

const static QString settingsFileName("context.dat");
const static QString airdropRequestsFileName("requests.dat");
const static QString setupDoneFileName("setup.dat");


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
    QString dataPath = ioutils::getAppDataPath("context");

    QFile file(dataPath + "/" + settingsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_7);

    int id = 0;
    in >> id;
    if (id<0x4783 || id>0x4787)
         return false;

    in >> receiveAccount;
    in >> currentAccountName;

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

    return true;
}


void AppContext::saveData() const {
    QString dataPath = ioutils::getAppDataPath("context");

    QString filePath = dataPath + "/" + settingsFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        control::MessageBox::message(nullptr,
              "ERROR",
              "Unable to save gui-wallet settings to " + filePath +
              "\nError: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);

    out << 0x4787;
    out << receiveAccount;
    out << currentAccountName;
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
}

void AppContext::setLogsEnabled(bool enabled) {
    if (enabled == logsEnabled)
        return;
    logsEnabled = enabled;
    saveData();
}


// AirdropRequests will handle differently
void AppContext::saveAirdropRequests( const QVector<state::AirdropRequests> & data ) {
    QString dataPath = ioutils::getAppDataPath("context");

    QString filePath = dataPath + "/" + airdropRequestsFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        control::MessageBox::message(nullptr,
                                     "ERROR",
                                     "Unable to aidrop requests to " + filePath +
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

    QString dataPath = ioutils::getAppDataPath("context");

    QFile file(dataPath + "/" + airdropRequestsFileName);
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

// First run for a new version flags support...
bool AppContext::isSetupDone(QString version) {
    QStringList lns = util::readTextFile(ioutils::getAppDataPath("context") + "/" + airdropRequestsFileName );
    return lns.size()>0 && lns[0]==version;
}

void AppContext::updateSetupDone(QString version) {
    util::writeTextFile(ioutils::getAppDataPath("context") + "/" + airdropRequestsFileName, QStringList{version} );
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


}

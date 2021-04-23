// Copyright 2020 The MWC Developers
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

#include "config_b.h"
#include "../core/Config.h"
#include "../build_version.h"
#include "../core/global.h"
#include "../state/state.h"
#include "../core/appcontext.h"
#include "../util/Process.h"
#include "../util/ioutils.h"
#include "../wallet/wallet.h"
#include "../state/state.h"
#include "../state/statemachine.h"

namespace bridge {

static wallet::Wallet * getWallet() { return state::getStateContext()->wallet; }
static core::AppContext * getAppContext() { return state::getStateContext()->appContext; }

Config::Config(QObject * parent) : QObject(parent) {}
Config::~Config() {}

// Build version for the app
QString Config::getBuildVersion() {
    return BUILD_VERSION;
}

// Get current Architecture with what app was build
QString Config::getBuildArch() {
    return util::getBuildArch();
}

QString Config::get_APP_NAME() {
    return mwc::get_APP_NAME();
}

QString Config::getAppDataPath(QString localPath) {
    QPair<bool,QString> res = ioutils::getAppDataPath(localPath);

    if (res.first)
        return res.second;

    return " " + res.second;
}

// Update config with foreign API settings.
void Config::saveForeignApiConfig(bool foreignApi,
                                      QString foreignApiAddress,
                                      QString tlsCertificateFile, QString tlsCertificateKey) {

    wallet::WalletConfig config = getWallet()->getWalletConfig();
    config.setForeignApi(foreignApi,
            foreignApiAddress,
            tlsCertificateFile, tlsCertificateKey);


    if (getWallet()->setWalletConfig(config, false)) {
        state::getStateContext()->stateMachine->executeFrom( state::STATE::NONE );
    }
}

// Request wallet instances. Returns the data as:
// <selected path_id>, < <path_id>, <instance name>, <network> >, ...  >
QVector<QString> Config::getWalletInstances(bool hasSeed) {
    QPair<QVector<QString>, int> instances = getAppContext()->getWalletInstances(hasSeed);

    // Expected that data is available
    Q_ASSERT(!instances.first.isEmpty());
    if (instances.first.isEmpty())
        return {};

    QVector<QString> result;
    result.push_back(instances.first[instances.second]);

    for ( const QString & wallet_local_path : instances.first ) {
        QVector<QString> nai = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(wallet_local_path, getAppContext());
        result.push_back(wallet_local_path);
        result.push_back(nai[2]);
        result.push_back(nai[0]);
    }
    return result;
}

// Request current wallet instance details.
// Returns the data as 4 items tuple:
// < <path_id>, <full_path>, <instance name>, <network> >
QVector<QString> Config::getCurrentWalletInstance() {
    QPair<QVector<QString>, int> instances = getAppContext()->getWalletInstances(false);
    QVector<QString> result;

    if (instances.first.isEmpty()) {
        result.resize(4); // wallet is not open yet
        return result;
    }


    QString walletLocalPath = instances.first[instances.second];
    result.push_back(walletLocalPath);
    result.push_back( ioutils::getAppDataPath(walletLocalPath).second);
    QVector<QString> nai = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(walletLocalPath, getAppContext());
    result.push_back(nai[2]);
    result.push_back(nai[0]);
    return result;
}

// Set this instance as active
void Config::setActiveInstance(QString instancePathId) {
    getAppContext()->setCurrentWalletInstance(instancePathId);
}

// Update instance name
void Config::updateActiveInstanceName(QString newInstanceName) {
    QString walletLocalPath = getAppContext()->getCurrentWalletInstance(true);
    QVector<QString> nai = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(walletLocalPath, getAppContext());
    wallet::WalletConfig::saveNetwork2DataPath(walletLocalPath, nai[0], nai[1], newInstanceName);
}


bool Config::isOnlineWallet() {
    return config::isOnlineWallet();
}

bool Config::isOnlineNode() {
    return config::isOnlineNode();
}


bool Config::isColdWallet() {
    return config::isColdWallet();
}

// if wallet using TLS for http connection
bool Config::hasTls() {
    return getWallet()->hasTls();
}

// Get wallet network name
QString Config::getNetwork() {
    return getWallet()->getWalletConfig().getNetwork();
}

// SendCoinsParams, number of corfirmations
int Config::getInputConfirmationNumber() {
    return getAppContext()->getSendCoinsParams().inputConfirmationNumber;
}
// SendCoinsParams, number of change outputs
int Config::getChangeOutputs() {
    return getAppContext()->getSendCoinsParams().changeOutputs;
}
// Update SendCoinsParams values
void Config::updateSendCoinsParams(int inputConfirmationNumber, int changeOutputs) {
    getAppContext()->setSendCoinsParams(core::SendCoinsParams(inputConfirmationNumber, changeOutputs));
}

// Wallet data Path as it is written in the configs
QString Config::getDataPath() {
    return getWallet()->getWalletConfig().getDataPath();
}
bool Config::hasForeignApi() {
    return getWallet()->getWalletConfig().hasForeignApi();
}
QString Config::getForeignApiAddress() {
    return getWallet()->getWalletConfig().foreignApiAddress;
}
QString Config::getTlsCertificateKey() {
    return getWallet()->getWalletConfig().tlsCertificateKey;
}
QString Config::getTlsCertificateFile() {
    return getWallet()->getWalletConfig().tlsCertificateFile;
}



double Config::getTimeoutMultiplier() {
    return config::getTimeoutMultiplier();
}

// Get URL of the block explorer for mwc network
QString Config::getBlockExplorerUrl(QString walletNetwork) {
    return (walletNetwork == "Mainnet") ? mwc::BLOCK_EXPLORER_URL_MAINNET
                                        : mwc::BLOCK_EXPLORER_URL_FLOONET;
}

// Get a column names by table id. If this data not found, return empty array
QVector<int> Config::getColumnsWidhts(QString tableId) {
    return getAppContext()->getIntVectorFor(tableId);
}
// Store column widhts
void Config::updateColumnsWidhts( QString tableId, QVector<int> widths ) {
    getAppContext()->updateIntVectorFor(tableId, widths);
}

// Get last saved path for that pathId.
QString Config::getPathFor(QString pathId, bool returnEmptyForNew) {
    return getAppContext()->getPathFor(pathId, returnEmptyForNew);
}
// Save last selected path by user
void Config::updatePathFor(QString pathId, QString path) {
    return getAppContext()->updatePathFor(pathId, path);
}

// return pair of values for every contact: [name, address]
QVector<QString> Config::getContactsAsPairs() {
    QVector<QString> result;
    for ( const core::ContactRecord & cr : getAppContext()->getContacts() ) {
        result.push_back(cr.name);
        result.push_back(cr.address);
    }
    return result;
}

// delete the contact. Return empty string on OK. Otherwise it has an error
QString Config::deleteContact(QString name, QString address) {
    auto res = getAppContext()->deleteContact( core::ContactRecord(name, address) );
    return res.first ? "" : res.second;
}

// add new contact. Return empty string on OK. Otherwise it has an error
QString Config::addContact(QString name, QString address) {
    auto res = getAppContext()->addContact(core::ContactRecord(name, address));
    return res.first ? "" : res.second;
}

// Update contact record. Return empty string on OK. Otherwise it has an error
QString Config::updateContact(QString oldName, QString oldAddress, QString newName, QString newAddress) {
    auto res = getAppContext()->updateContact( core::ContactRecord(oldName, oldAddress), core::ContactRecord(newName, newAddress) );
    return res.first ? "" : res.second;
}

// Return true if current setting to show all outputs
bool Config::isShowOutputAll() {
    return getAppContext()->isShowOutputAll();
}
// Update 'show all outputs' setting value
void Config::setShowOutputAll(bool show) {
    getAppContext()->setShowOutputAll( show );
}

// return true if locking is enabled
bool Config::isLockOutputEnabled() {
    return getAppContext()->isLockOutputEnabled();
}

// Check if this output is locked
bool Config::isLockedOutput(QString outputCommitment) {
    return getAppContext()->isLockedOutputs(outputCommitment).first;
}
// Check if this output is locked
void Config::setLockedOutput(bool isLocked, QString outputCommitment) {
    getAppContext()->setLockedOutput(outputCommitment, isLocked, "");
}


// Read a note for this commitment
QString Config::getOutputNote( QString outputCommitment) {
    return getAppContext()->getNote("c_"+outputCommitment);
}
// Delete note fo this commit
void Config::deleteOutputNote( QString outputCommitment) {
    getAppContext()->deleteNote("c_"+outputCommitment);
}
// Update the note for this commit
void Config::updateOutputNote( QString outputCommitment, QString note) {
    getAppContext()->updateNote( "c_"+outputCommitment, note);
}
// Read a note for this transaction
QString Config::getTxNote(QString txUuid) {
    return getAppContext()->getNote("tx_"+txUuid);
}
// Delete note fo this transaction
void Config::deleteTxNote(QString txUuid) {
    getAppContext()->deleteNote("tx_"+txUuid);
}
// Update the note for this transaction
void Config::updateTxNote(QString txUuid, QString note) {
    getAppContext()->updateNote( "tx_"+txUuid, note);
}

// Read a note for this commitment
QString Config::getSwapNote(QString swapId) {
    return getAppContext()->getNote("swap_"+swapId);
}

// Update the note for this commit
void Config::updateSwapNote(QString swapId, QString note) {
    if (note.isEmpty())
        getAppContext()->deleteNote("swap_"+swapId);
    else
        getAppContext()->updateNote("swap_"+swapId, note);
}

// Check if 'fluff' flag is set
bool Config::isFluffSet() {
    return getAppContext()->isFluffSet();
}
// Update fluff flag value
void Config::setFluff(bool fluffSetting) {
    getAppContext()->setFluff(fluffSetting);
}

// Get 'generate proof' flag value
bool Config::getGenerateProof() {
    return getAppContext()->getGenerateProof();
}

// Update 'generate proof' flag value
void Config::setGenerateProof(bool generate) {
    getAppContext()->setGenerateProof(generate);
}

// Check if notification windows are enabled
bool Config::getNotificationWindowsEnabled() {
    return getAppContext()->getNotificationWindowsEnabled();
}

// Check if 'fluff' flag is set
int Config::getSwapTabSelection() {
    return getAppContext()->getSwapTabSelection();
}

// Update fluff flag value
void Config::setSwapTabSelection(int selection) {
    getAppContext()->setSwapTabSelection(selection);
}

// Backup dir for swap trades.
QString Config::getSwapBackupDir() {
    return getAppContext()->getSwapBackupDir();
}
void Config::setSwapBackupDir(QString backupDir) {
    getAppContext()->setSwapBackupDir(backupDir);
}

int  Config::getSwapBackStatus(QString swapId) {
    return getAppContext()->getSwapBackStatus(swapId);
}

void Config::setSwapBackStatus(QString swapId, int status) {
    getAppContext()->setSwapBackStatus(swapId, status);
}

int  Config::getMaxBackupStatus(QString swapId, int status) {
    return getAppContext()->getMaxBackupStatus(swapId, status);
}

bool Config::isTradeAccepted(QString swapId) {
    return getAppContext()->isTradeAccepted(swapId);
}

void Config::setTradeAcceptedFlag(QString swapId, bool accepted) {
    getAppContext()->setTradeAcceptedFlag(swapId, accepted);
}

// Setting is MWC Node prohibited to use TOR
bool Config::getNoTorForEmbeddedNode() {
    return getAppContext()->getNoTorForEmbeddedNode();
}
void Config::setNoTorForEmbeddedNode(bool noTor) {
    getAppContext()->setNoTorForEmbeddedNode(noTor);
}

// Slatepack format, selected value
// Slatepack format, selected value
int Config::getSendMethod() {
    return getAppContext()->getSendMethod();
}
void Config::setSendMethod(int method) {
    getAppContext()->setSendMethod(bridge::SEND_SELECTED_METHOD(method));
}

// Lock outputs for slatepacks send
bool Config::getSendLockOutput() {
    return getAppContext()->getSendLockOutput();
}

void Config::setSendLockOutput(bool lock) {
    getAppContext()->setSendLockOutput(lock);
}

int Config::getMktPlaceSelectedBtn() {
    return getAppContext()->getMktPlaceSelectedBtn();
}

void Config::setMktPlaceSelectedBtn(int btn) {
    getAppContext()->setMktPlaceSelectedBtn(btn);
}


double Config::getSwapMktMinFeeLevel() {
    return getAppContext()->getSwapMktMinFeeLevel();
}

int Config::getSwapMktSelling() {
    return getAppContext()->getSwapMktSelling();
}

QString Config::getSwapMktCurrency() {
    return getAppContext()->getSwapMktCurrency();
}

void Config::setSwapMktMinFeeLevel(double fee) {
    getAppContext()->setSwapMktMinFeeLevel(fee);
}

void Config::setSwapMktSelling(int buySellBoth) {
    getAppContext()->setSwapMktSelling(buySellBoth);
}

void Config::setSwapMktCurrency(QString currency) {
    getAppContext()->setSwapMktCurrency(currency);
}


}

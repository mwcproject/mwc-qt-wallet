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

#ifndef MWC_QT_WALLET_CONFIG_B_H
#define MWC_QT_WALLET_CONFIG_B_H

#include <QObject>

namespace bridge {

class Config : public QObject {
Q_OBJECT
public:
    explicit Config(QObject * parent = nullptr);
    ~Config();

    // Build version for the app
    Q_INVOKABLE QString getBuildVersion();
    // Get current Architecture with what app was build
    Q_INVOKABLE QString getBuildArch();
    // Get app name
    Q_INVOKABLE QString get_APP_NAME();

    // Get data path for that local dorectory.
    // Return:  path is case of seccess.
    //          ' 'Error message.
    // Error message will be started with ' ' - space
    Q_INVOKABLE QString getAppDataPath(QString localPath = "");

    // Update config with foreign API settings.
    Q_INVOKABLE void saveForeignApiConfig(bool foreignApi,
                                          QString foreignApiAddress,
                                          QString tlsCertificateFile, QString tlsCertificateKey);

    // Request wallet instances.
    // hasSeed - true if needed initialized wallet instances.
    // Returns the data as:
    // <selected path_id>, < <path_id>, <instance name>, <network> >, ...  >
    Q_INVOKABLE QVector<QString> getWalletInstances(bool hasSeed);
    // Request current wallet instance details.
    // Returns the data as 4 items tuple:
    // < <path_id>, <full_path>, <instance name>, <network> >
    Q_INVOKABLE QVector<QString> getCurrentWalletInstance();

    // Set this instance as active
    Q_INVOKABLE void setActiveInstance(QString instancePathId);
    // Update instance name
    Q_INVOKABLE void updateActiveInstanceName(QString newInstanceName);

    // Wallet running mode. Mobile can support OnlineWallet only.
    Q_INVOKABLE bool isOnlineWallet();
    Q_INVOKABLE bool isOnlineNode();
    Q_INVOKABLE bool isColdWallet();

    // ---------------- Wallet Config data -----------------
    // if wallet using TLS for http connection
    Q_INVOKABLE bool hasTls();
    // Wallet data Path as it is written in the configs
    Q_INVOKABLE QString getDataPath();
    Q_INVOKABLE bool hasForeignApi();
    Q_INVOKABLE QString getForeignApiAddress();
    Q_INVOKABLE QString getTlsCertificateKey();
    Q_INVOKABLE QString getTlsCertificateFile();
    // Get wallet network name
    Q_INVOKABLE QString getNetwork();

    // SendCoinsParams, number of corfirmations
    Q_INVOKABLE int getInputConfirmationNumber();
    // SendCoinsParams, number of change outputs
    Q_INVOKABLE int getChangeOutputs();
    // Update SendCoinsParams values
    Q_INVOKABLE void updateSendCoinsParams(int inputConfirmationNumber, int changeOutputs);

    // Global multiplier for all waiting operations. Slower host, larger this number
    Q_INVOKABLE double getTimeoutMultiplier();

    // Get URL of the block explorer for mwc network
    Q_INVOKABLE QString getBlockExplorerUrl(QString walletNetwork);

    // Get a column names by table id. If this data not found, return empty array
    Q_INVOKABLE QVector<int> getColumnsWidhts(QString tableId);
    // Store column widhts
    Q_INVOKABLE void updateColumnsWidhts( QString tableId, QVector<int> widths );

    // Get last saved path for that pathId.
    Q_INVOKABLE QString getPathFor(QString pathId, bool returnEmptyForNew = false);
    // Save last selected path by user
    Q_INVOKABLE void updatePathFor(QString pathId, QString path);

    // return pair of values for every contact: [name, address]
    Q_INVOKABLE QVector<QString> getContactsAsPairs();
    // delete the contact. Return empty string on OK. Otherwise it has an error
    Q_INVOKABLE QString deleteContact(QString name, QString address);
    // add new contact. Return empty string on OK. Otherwise it has an error
    Q_INVOKABLE QString addContact(QString name, QString address);
    // Update contact record. Return empty string on OK. Otherwise it has an error
    Q_INVOKABLE QString updateContact(QString oldName, QString oldAddress, QString newName, QString newAddress);

    // Return true if current setting to show all outputs
    Q_INVOKABLE bool isShowOutputAll();
    // Update 'show all outputs' setting value
    Q_INVOKABLE void setShowOutputAll(bool show);

    // return true if locking is enabled
    Q_INVOKABLE bool isLockOutputEnabled();
    // Check if this output is locked
    Q_INVOKABLE bool isLockedOutput(QString outputCommitment);
    // Check if this output is locked
    Q_INVOKABLE void setLockedOutput(bool isLocked, QString outputCommitment);

    // Read a note for this commitment
    Q_INVOKABLE QString getOutputNote(QString outputCommitment);
    // Delete note fo this commit
    Q_INVOKABLE void deleteOutputNote(QString outputCommitment);
    // Update the note for this commit
    Q_INVOKABLE void updateOutputNote(QString outputCommitment, QString note);
    // Read a note for this transaction
    Q_INVOKABLE QString getTxNote(QString txUuid);
    // Delete note fo this transaction
    Q_INVOKABLE void deleteTxNote(QString txUuid);
    // Update the note for this transaction
    Q_INVOKABLE void updateTxNote(QString txUuid, QString note);
    // Read a note for this commitment
    Q_INVOKABLE QString getSwapNote(QString swapId);
    // Update the note for this commit
    Q_INVOKABLE void updateSwapNote(QString swapId, QString note);

    // Check if 'fluff' flag is set
    Q_INVOKABLE bool isFluffSet();
    // Update fluff flag value
    Q_INVOKABLE void setFluff(bool fluffSetting);

    // Get 'generate proof' flag value
    Q_INVOKABLE bool getGenerateProof();
    // Update 'generate proof' flag value
    Q_INVOKABLE void setGenerateProof(bool generate);

    // Check if notification windows are enabled
    Q_INVOKABLE bool getNotificationWindowsEnabled();

    // Check if 'fluff' flag is set
    Q_INVOKABLE int getSwapTabSelection();
    // Update fluff flag value
    Q_INVOKABLE void setSwapTabSelection(int selection);

    // Backup dir for swap trades.
    Q_INVOKABLE QString getSwapBackupDir();
    Q_INVOKABLE void setSwapBackupDir(QString backupDir);

    // Backup status. We will ask user to do a backup for this swap.
    Q_INVOKABLE int  getSwapBackStatus(QString swapId);
    Q_INVOKABLE void setSwapBackStatus(QString swapId, int status);

    Q_INVOKABLE int  getMaxBackupStatus(QString swapId, int status);

    Q_INVOKABLE bool isTradeAccepted(QString swapId);
    Q_INVOKABLE void setTradeAcceptedFlag(QString swapId, bool accepted);

    // Setting is MWC Node prohibited to use TOR
    Q_INVOKABLE bool getNoTorForEmbeddedNode();
    Q_INVOKABLE void setNoTorForEmbeddedNode(bool noTor);

    // Slatepack format, selected value
    Q_INVOKABLE int getSendMethod();
    Q_INVOKABLE void setSendMethod(int method);

    // Lock outputs for slatepacks send
    Q_INVOKABLE bool getSendLockOutput();
    Q_INVOKABLE void setSendLockOutput(bool lock);

    Q_INVOKABLE int getMktPlaceSelectedBtn();
    Q_INVOKABLE void setMktPlaceSelectedBtn(int btn);

    Q_INVOKABLE double getSwapMktMinFeeLevel();
    Q_INVOKABLE int getSwapMktSelling();
    Q_INVOKABLE QString getSwapMktCurrency();
    Q_INVOKABLE void setSwapMktMinFeeLevel(double fee);
    Q_INVOKABLE void setSwapMktSelling(int buySellBoth);
    Q_INVOKABLE void setSwapMktCurrency(QString currency);

    Q_INVOKABLE QString getTorBridgeConection();
    Q_INVOKABLE QString getTorClientOptions();

    Q_INVOKABLE void setTorBridgeConectionClientOptions(QString bridge, QString client);
};

}

#endif //MWC_QT_WALLET_CONFIG_B_H

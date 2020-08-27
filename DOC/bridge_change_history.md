# About #

In this document we are tracking the changes that are made on the Bridges level. 

It is needed for Mobils and Desktop integrations.

# August 9, 2020 #

### Changed at Bridge: InitAccount 

From:

```
// newWalletChoice: see values at state::InitAccount::NEW_WALLET_CHOICE
// network: see values at state::InitAccount::MWC_NETWORK
Q_INVOKABLE void submitWalletCreateChoices(int newWalletChoice, int network);
```
To: 
```
// network: see values at state::InitAccount::MWC_NETWORK
// newInstanceName: Name of the new wallet instance
Q_INVOKABLE void submitWalletCreateChoices(int network, QString newInstanceName);
```

Note: QML code is updated at  InitAccount.qml    Line: 326


### Change at Bridge:  WalletConfig

**Deleted method 'setDataPathWithNetwork'**

**Add method:**
```
// Specify if the Online Node will run as a mainnet.
Q_INVOKABLE void setOnlineNodeRunsMainNetwork(bool isMainNet);
```

### Change at Bridge:  WalletConfig

From:
```
Q_INVOKABLE bool updateWalletConfig(QString network, QString dataPath,
                    QString mwcmqsDomain, QString keyBasePath, bool need2updateGuiSize);
```
To:
```
Q_INVOKABLE bool updateWalletConfig( QString mwcmqsDomain, QString keyBasePath, bool need2updateGuiSize );
```

Note: Wallet data path & network managed with different methods. Both those methods not releated to Mobile.

### Created a new Bridge  StartWallet

New method:
```
// Create a new Wallet Instance
// path - wallet data location or empty for the next available_path (new wallet case only)
// restoreWallet - true if the next new wallet will be restore from the seed.
Q_INVOKABLE void createNewWalletInstance(QString path, bool restoreWallet);
```
Note: Mobile will need that new bridge and method to support of creation of a new instance.  Mobile should specify path as empty string ("")

### Change at Bridge:  Config

**Deleted metod: "doesSeedExist"**

**Deleted metod: "readNetworkArchFromDataPath"**

**Deleted metod: "saveNetwork2DataPath"**

Note: New wallet managed differently now, more simple way. Mobile shouldn't use deleted methods yet.


**Add new methods:**
```
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
```
Note: Mobile will need to adopt them for instance management.



# August 26, 2020 #

Add a new bridge: Swap

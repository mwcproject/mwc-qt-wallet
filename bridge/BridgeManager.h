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

#ifndef MWC_QT_WALLET_BRIDGEMANAGER_H
#define MWC_QT_WALLET_BRIDGEMANAGER_H

#include <QSet>

namespace bridge {

class ProgressWnd;
class InputPassword;
class Outputs;
class Receive;
class Finalize;
class FileTransaction;
class Listening;
class Send;
class Transactions;
class Hodl;
class Accounts;
class AccountTransfer;
class Airdrop;
class NodeInfo;
class Events;
class NewSeed;
class WalletConfig;
class CoreWindow;
class SelectMode;

// Because many instances of bridges might exist, we need some place to map them.
// Please use getBridgeManager to access a singletone.
class BridgeManager {
public:
    void addProgressWnd( bridge::ProgressWnd * b ) {progressWnd += b;}
    void removeProgressWnd( bridge::ProgressWnd * b ) {progressWnd -= b;}
    const QSet<ProgressWnd*> & getProgressWnd() const {return progressWnd;}

    void addInputPassword( bridge::InputPassword * b ) {inputPassword += b;}
    void removeInputPassword( bridge::InputPassword * b ) {inputPassword -= b;}
    const QSet<InputPassword*> & getInputPassword() const {return inputPassword;}

    void addOutputs( bridge::Outputs * b ) {outputs += b;}
    void removeOutputs( bridge::Outputs * b ) {outputs -= b;}
    const QSet<Outputs*> & getOutputs() const {return outputs;}

    void addReceive( bridge::Receive * b ) {receive += b;}
    void removeReceive( bridge::Receive * b ) {receive -= b;}
    const QSet<Receive*> & getReceive() const {return receive;}

    void addFinalize( bridge::Finalize * b ) {finalize += b;}
    void removeFinalize( bridge::Finalize * b ) {finalize -= b;}
    const QSet<Finalize*> & getFinalize() const {return finalize;}

    void addFileTransaction( bridge::FileTransaction * b ) {fileTransaction += b;}
    void removeFileTransaction( bridge::FileTransaction * b ) {fileTransaction -= b;}
    const QSet<FileTransaction*> & getFileTransaction() const {return fileTransaction;}

    void addListening( bridge::Listening * b ) {listening += b;}
    void removeListening( bridge::Listening * b ) {listening -= b;}
    const QSet<Listening*> & getListening() const {return listening;}

    void addSend( bridge::Send * b ) {send += b;}
    void removeSend( bridge::Send * b ) {send -= b;}
    const QSet<Send*> & getSend() const {return send;}

    void addTransactions( bridge::Transactions * b ) {transactions += b;}
    void removeTransactions( bridge::Transactions * b ) {transactions -= b;}
    const QSet<Transactions*> & getTransactions() const {return transactions;}

    void addHodl( bridge::Hodl * b ) {hodl += b;}
    void removeHodl( bridge::Hodl * b ) {hodl -= b;}
    const QSet<Hodl*> & getHodl() const {return hodl;}

    void addAccounts( bridge::Accounts * b ) {accounts += b;}
    void removeAccounts( bridge::Accounts * b ) {accounts -= b;}
    const QSet<Accounts*> & getAccounts() const {return accounts;}

    void addAccountTransfer( bridge::AccountTransfer * b ) {accountTransfer += b;}
    void removeAccountTransfer( bridge::AccountTransfer * b ) {accountTransfer -= b;}
    const QSet<AccountTransfer*> & getAccountTransfer() const {return accountTransfer;}

    void addAirdrop( bridge::Airdrop * b ) {airdrop += b;}
    void removeAirdrop( bridge::Airdrop * b ) {airdrop -= b;}
    const QSet<Airdrop*> & getAirdrop() const {return airdrop;}

    void addNodeInfo( bridge::NodeInfo * b ) {nodeInfo += b;}
    void removeNodeInfo( bridge::NodeInfo * b ) {nodeInfo -= b;}
    const QSet<NodeInfo*> & getNodeInfo() const {return nodeInfo;}

    void addEvents( bridge::Events * b ) {events += b;}
    void removeEvents( bridge::Events * b ) {events -= b;}
    const QSet<Events*> & getEvents() const {return events;}

    void addNewSeed( bridge::NewSeed * b ) {newSeed += b;}
    void removeNewSeed( bridge::NewSeed * b ) {newSeed -= b;}
    const QSet<NewSeed*> & getNewSeed() const {return newSeed;}

    void addWalletConfig( bridge::WalletConfig * b ) {walletConfig += b;}
    void removeWalletConfig( bridge::WalletConfig * b ) {walletConfig -= b;}
    const QSet<WalletConfig*> & getWalletConfig() const {return walletConfig;}

    void addCoreWindow( bridge::CoreWindow * b ) {coreWindow += b;}
    void removeCoreWindow( bridge::CoreWindow * b ) {coreWindow -= b;}
    const QSet<CoreWindow*> & getCoreWindow() const {return coreWindow;}

    void addSelectMode( bridge::SelectMode * b ) {selectMode += b;}
    void removeSelectMode( bridge::SelectMode * b ) {selectMode -= b;}
    const QSet<SelectMode*> & getSelectMode() const {return selectMode;}
private:
    QSet<ProgressWnd*>      progressWnd;
    QSet<InputPassword*>    inputPassword;
    QSet<Outputs*>          outputs;
    QSet<Receive*>          receive;
    QSet<Finalize*>         finalize;
    QSet<FileTransaction*>  fileTransaction;
    QSet<Listening*>        listening;
    QSet<Send*>             send;
    QSet<Transactions*>     transactions;
    QSet<Hodl*>             hodl;
    QSet<Accounts*>         accounts;
    QSet<AccountTransfer*>  accountTransfer;
    QSet<Airdrop*>          airdrop;
    QSet<NodeInfo*>         nodeInfo;
    QSet<Events*>           events;
    QSet<NewSeed*>          newSeed;
    QSet<WalletConfig*>     walletConfig;
    QSet<CoreWindow*>       coreWindow;
    QSet<SelectMode*>       selectMode;

};

BridgeManager * getBridgeManager();

}

#endif //MWC_QT_WALLET_BRIDGEMANAGER_H

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

#ifndef MWC_QT_WALLET_IWALLETWNDMANANGER_H
#define MWC_QT_WALLET_IWALLETWNDMANANGER_H

#include <QString>
#include "../util/Json.h"
#include "../wallet/wallet.h"

// Interface for all Windows manipulation that UI can use.
// Desktop and Mobile has it's own implementation.
// NOTE: All XxxxxxDlg methods MUST be modal. Mobile version should 'simulate' that

namespace core {

class WndManager {
public:
    enum class RETURN_CODE {BTN1, BTN2};

    // Page Ids. Needed to query what page is active or visible.
    enum class PAGE_WND_ID {NONE, PAGE_INPUT_PASSWORD, PAGE_INIT_ACCOUNT, PAGE_ENTER_SEED, PAGE_NEW_SEED, PAGE_NEW_SEED_TEST, PAGE_PROGRESS_WND, PAGE_OUTPUTS};

    WndManager() {}
    virtual ~WndManager() {}

    //------------------  Message boxes ---------------
    //
    virtual void messageTextDlg( QString title, QString message, double widthScale = 1.0) = 0;
    virtual void messageHtmlDlg( QString title, QString message, double widthScale = 1.0) = 0;
    // Two button box
    virtual RETURN_CODE questionTextDlg( QString title, QString message, QString btn1, QString btn2, bool default1=false, bool default2=true, double widthScale = 1.0 ) = 0;
    virtual RETURN_CODE questionHTMLDlg( QString title, QString message, QString btn1, QString btn2, bool default1=false, bool default2=true, double widthScale = 1.0 ) = 0;
    // Password accepted as a HASH. EMpty String mean that no password is set.
    // After return, passwordHash value will have input raw Password value. So it can be user for wallet
    virtual RETURN_CODE questionTextDlg( QString title, QString message, QString btn1, QString btn2, bool default1, bool default2, double widthScale, QString & passwordHash, RETURN_CODE blockButton ) = 0;

    // QFileDialog::getSaveFileName call
    virtual QString getSaveFileName(const QString &caption, const QString &dir, const QString &filter) = 0;

    // Ask for confirmation
    virtual bool sendConfirmationDlg( QString title, QString message, double widthScale, QString passwordHash ) = 0;

    //---------------- Pages ------------------------
    virtual void pageNewWallet() = 0;
    virtual void pageInputPassword(QString pageTitle, bool lockMode) = 0;
    virtual void pageInitAccount() = 0;
    virtual void pageEnterSeed() = 0;
    virtual void pageNewSeed(QString pageTitle, QVector<QString> seed, bool hideSubmitButton = false) = 0;
    virtual void pageNewSeedTest(int wordIndex) = 0;
    virtual void pageProgressWnd(QString pageTitle, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) = 0;
    virtual void pageOutputs() = 0;
    virtual void pageFileTransaction(QString pageTitle, QString callerId,
                       const QString & fileName, const util::FileTransactionInfo & transInfo,
                       int nodeHeight,
                       QString transactionType, QString processButtonName) = 0;
    virtual void pageRecieve() = 0;
    virtual void pageListening() = 0;
    virtual void pageFinalize() = 0;
    virtual void pageSendStarting() = 0;
    virtual void pageSendOnline( QString selectedAccount, int64_t amount ) = 0;
    virtual void pageSendOffline( QString selectedAccount, int64_t amount ) = 0;
    virtual void pageTransactions() = 0;

    virtual void pageHodl() = 0;
    virtual void pageHodlNode() = 0;
    virtual void pageHodlCold() = 0;
    virtual void pageHodlClaim(QString walletHash) = 0;
    // return true(first) if press OK and input signature value(second)
    virtual QPair<bool, QString> hodlGetSignatureDlg(QString challenge) = 0;
    virtual void pageAccounts() = 0;
    virtual void pageAccountTransfer() = 0;
    virtual void pageAirdrop() = 0;
    virtual void pageAirdropForBTC(QString btcAddress, QString challenge, QString identifier) = 0;
    virtual void pageNodeInfo() = 0;
    virtual void pageContacts() = 0;
    virtual void pageEvents() = 0;
    virtual void pageWalletConfig() = 0;
    virtual void pageNodeConfig() = 0;
    virtual void pageSelectMode() = 0;
};

void setWndManager(WndManager * mgr);
WndManager *getWndManager();

}

#endif //MWC_QT_WALLET_IWALLETWNDMANANGER_H

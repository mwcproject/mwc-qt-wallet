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
    virtual RETURN_CODE questionTextDlg( QString title, QString message, QString btn1, QString btn2,
                QString btn1Tooltip, QString btn2Tooltip,
                bool default1=false, bool default2=true, double widthScale = 1.0, int *ttl_blocks = 0) = 0;
    virtual RETURN_CODE questionHTMLDlg( QString title, QString message, QString btn1, QString btn2,
                QString btn1Tooltip, QString btn2Tooltip,
                bool default1=false, bool default2=true, double widthScale = 1.0 ) = 0;
    // Password accepted as a HASH. Empty String mean that no password is set.
    // After return, passwordHash value will have input raw Password value. So it can be user for wallet
    virtual RETURN_CODE questionTextDlg( QString title, QString message, QString btn1, QString btn2,
            QString btn1Tooltip, QString btn2Tooltip,
            bool default1, bool default2, double widthScale, QString & passwordHash, RETURN_CODE blockButton, int *ttl_blocks ) = 0;

    // QFileDialog::getSaveFileName call
    virtual QString getSaveFileName(const QString &caption, const QString &dir, const QString &filter) = 0;
    virtual QString getOpenFileName(const QString &caption, const QString &dir, const QString &filter) = 0;

    // Ask for confirmation
    virtual bool sendConfirmationDlg( QString title, QString message, double widthScale, QString passwordHash ) = 0;

    // Stopping wallet message
    virtual void showWalletStoppingMessage(int taskTimeout) = 0;
    virtual void hideWalletStoppingMessage() = 0;

    //---------------- Pages ------------------------
    virtual void pageInitFirstTime() = 0;
    virtual void pageInputPassword(QString pageTitle, bool lockMode) = 0;
    virtual void pageInitAccount(QString path, bool restoredFromSeed) = 0;
    virtual void pageEnterSeed() = 0;
    virtual void pageNewSeed(QString pageTitle, QVector<QString> seed, bool hideSubmitButton = false) = 0;
    virtual void pageNewSeedTest(int wordIndex) = 0;
    virtual void pageProgressWnd(QString pageTitle, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) = 0;
    virtual void pageOutputs() = 0;
    virtual void pageFileTransactionReceive(QString pageTitle,
                       const QString & fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
                       int nodeHeight) = 0;
    virtual void pageFileTransactionFinalize(QString pageTitle,
                                     const QString & fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
                                     int nodeHeight) = 0;
    virtual void pageRecieve() = 0;
    virtual void pageListening() = 0;
    virtual void pageFinalize() = 0;
    virtual void pageSendStarting() = 0;
    virtual void pageSendOnline( QString selectedAccount, int64_t amount ) = 0;
    virtual void pageSendFile( QString selectedAccount, int64_t amount ) = 0;
    virtual void pageSendSlatepack( QString selectedAccount, int64_t amount ) = 0;
    virtual void pageTransactions() = 0;
    // slatepack - slatepack string value to show.
    // backStateId - state ID of the caller. On 'back' will switch to this state Id
    virtual void pageShowSlatepack(QString slatepack, int backStateId, QString txExtension, bool enableFinalize) = 0;

    virtual void pageAccounts() = 0;
    virtual void pageAccountTransfer() = 0;
    virtual void pageNodeInfo() = 0;
    virtual void pageContacts() = 0;
    virtual void pageEvents() = 0;
    virtual void pageWalletConfig() = 0;
    virtual void pageNodeConfig() = 0;
    virtual void pageSelectMode() = 0;


    // Mobile specific pages
    virtual void pageWalletHome() = 0;
    virtual void pageWalletSettings() = 0;
    virtual void pageAccountOptions() = 0;

    // Swap pages
    virtual void pageSwapList(bool selectIncoming, bool selectOutgoing, bool selectBackup) = 0;
    virtual void pageSwapNew1() = 0;
    virtual void pageSwapNew2() = 0;
    virtual void pageSwapNew3() = 0;
    virtual void pageSwapEdit(QString swapId, QString stateCmd) = 0;
    virtual void pageSwapTradeDetails(QString swapId) = 0;

    // Swap marketplace, if selectMyOffers==true,  my offers will be selected.
    // Otherwise last known tab
    virtual void pageMarketplace(bool selectMyOffers, bool selectFee) = 0;
    // new offer of myMsgId is empty. Otherwise it is running offer update
    virtual void pageNewUpdateOffer(QString myMsgId) = 0;

    // Show dialog that enforce setup of backup directory
    virtual void showSwapBackupDlg() = 0;

    // Show transaction for integrity fee page
    virtual void pageTransactionFee() = 0;
};

void setWndManager(WndManager * mgr);
WndManager *getWndManager();

}

#endif //MWC_QT_WALLET_IWALLETWNDMANANGER_H

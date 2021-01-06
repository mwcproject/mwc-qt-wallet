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

#ifndef MWC_QT_WALLET_MOBILEWNDMANAGER_H
#define MWC_QT_WALLET_MOBILEWNDMANAGER_H

#include "../core/WndManager.h"

class QQmlApplicationEngine;

namespace core {

class MobileWndManager : public WndManager {
public:
    MobileWndManager() {};
    virtual ~MobileWndManager() {};

    void init(QQmlApplicationEngine * engine);


    //------------------  Message boxes ---------------
    //
    virtual void messageTextDlg( QString title, QString message, double widthScale = 1.0) override;
    virtual void messageHtmlDlg( QString title, QString message, double widthScale = 1.0) override;
    // Two button box
    virtual RETURN_CODE questionTextDlg( QString title, QString message,
                                         QString btn1, QString btn2,
                                         QString btn1Tooltip, QString btn2Tooltip,
                                         bool default1=false, bool default2=true, double widthScale = 1.0, int *ttl_blocks = nullptr ) override;
    virtual RETURN_CODE questionHTMLDlg( QString title, QString message,
                                         QString btn1, QString btn2,
                                         QString btn1Tooltip, QString btn2Tooltip,
                                         bool default1=false, bool default2=true, double widthScale = 1.0 ) override;

    // Password accepted as a HASH. EMpty String mean that no password is set.
    // After return, passwordHash value will have input raw Password value. So it can be user for wallet
    virtual RETURN_CODE questionTextDlg( QString title, QString message, QString btn1, QString btn2,
                                         QString btn1Tooltip, QString btn2Tooltip,
                                         bool default1, bool default2, double widthScale, QString & passwordHash, RETURN_CODE blockButton, int *ttl_blocks ) override;

    // QFileDialog::getSaveFileName call
    virtual QString getSaveFileName(const QString &caption, const QString &dir, const QString &filter) override;
    virtual QString getLoadFileName(const QString &caption, const QString &dir, const QString &filter) override;

    // Ask for confirmation
    virtual bool sendConfirmationDlg( QString title, QString message, double widthScale, QString passwordHash ) override;

    // Stopping wallet message
    virtual void showWalletStoppingMessage(int taskTimeout) override;
    virtual void hideWalletStoppingMessage() override;

    //---------------- Pages ------------------------
    virtual void pageInitFirstTime() override;
    virtual void pageInputPassword(QString pageTitle, bool lockMode) override;
    virtual void pageInitAccount(QString path, bool restoredFromSeed) override;
    virtual void pageEnterSeed() override;
    virtual void pageNewSeed(QString pageTitle, QVector<QString> seed, bool hideSubmitButton = false) override;
    virtual void pageNewSeedTest(int wordIndex) override;
    virtual void pageProgressWnd(QString pageTitle, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) override;
    virtual void pageOutputs() override;
    virtual void pageFileTransaction(QString pageTitle, QString callerId,
                                     const QString & fileName, const util::FileTransactionInfo & transInfo,
                                     int nodeHeight,
                                     QString transactionType, QString processButtonName) override;
    virtual void pageRecieve() override;
    virtual void pageListening() override;
    virtual void pageFinalize() override;
    virtual void pageSendStarting() override;
    virtual void pageSendOnline( QString selectedAccount, int64_t amount ) override;
    virtual void pageSendOffline( QString selectedAccount, int64_t amount ) override;
    virtual void pageTransactions() override;
    // slatepack - slatepack string value to show.
    // backStateId - state ID of the caller. On 'back' will switch to this state Id
    virtual void pageShowSlatepack(QString slatepack, int backStateId, QString txExtension) override;

    virtual void pageAccounts() override;
    virtual void pageAccountTransfer() override;
    virtual void pageNodeInfo() override;
    virtual void pageContacts() override;
    virtual void pageEvents() override;
    virtual void pageWalletConfig() override;
    virtual void pageNodeConfig() override;
    virtual void pageSelectMode() override;

    // Mobile specific pages
    virtual void pageWalletHome() override;
    virtual void pageWalletSettings() override;
    virtual void pageAccountOptions() override;

    // Swap pages
    virtual void pageSwapList() override;
    virtual void pageSwapNew1() override;
    virtual void pageSwapNew2() override;
    virtual void pageSwapNew3() override;
    virtual void pageSwapEdit(QString swapId, QString stateCmd) override;
    virtual void pageSwapTradeDetails(QString swapId) override;

    virtual void showBackupDlg(QString swapId, int backupId) override;
private:
    QQmlApplicationEngine * engine = nullptr;
    QObject * mainWindow = nullptr;
};

}



#endif //MWC_QT_WALLET_MOBILEWNDMANAGER_H

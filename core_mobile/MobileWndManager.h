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
                                         bool default1=false, bool default2=true, double widthScale = 1.0 ) override;
    virtual RETURN_CODE questionHTMLDlg( QString title, QString message,
                                         QString btn1, QString btn2,
                                         QString btn1Tooltip, QString btn2Tooltip,
                                         bool default1=false, bool default2=true, double widthScale = 1.0 ) override;

    // Password accepted as a HASH. EMpty String mean that no password is set.
    // After return, passwordHash value will have input raw Password value. So it can be user for wallet
    RETURN_CODE questionTextDlg( QString title, QString message, QString btn1, QString btn2,
                                 QString btn1Tooltip, QString btn2Tooltip,
                                 bool default1, bool default2, double widthScale, QString & passwordHash, RETURN_CODE blockButton, int *ttl_blocks );

    // QFileDialog::getSaveFileName call
    virtual QString getSaveFileName(const QString &caption, const QString &dir, const QString &filter) override;
    virtual QString getOpenFileName(const QString &caption, const QString &dir, const QString &filter) override;

    // Ask for confirmation
    virtual bool sendConfirmationSlatepackDlg( QString title, QString messageBody, double widthScale, int inputsNum, int * ttl ) override;
    virtual bool sendConfirmationDlg( QString title, QString message, double widthScale, int inputsNum ) override;
    bool sendConfirmationDlg( QString title, QString message, double widthScale );

    // Stopping wallet message
    void showWalletStoppingMessage(int taskTimeout);
    void hideWalletStoppingMessage();

    //---------------- Pages ------------------------
    virtual void pageInitFirstTime() override;
    virtual void pageInputPassword(QString pageTitle, bool lockMode) override;
    virtual void pageInitAccount(QString path, bool restoredFromSeed) override;
    virtual void pageSeedLength() override;
    virtual void pageEnterSeed() override;
    virtual void pageNewSeed(QString pageTitle, QVector<QString> seed, bool hideSubmitButton = false) override;
    virtual void pageNewSeedTest(int wordIndex) override;
    virtual void pageProgressWnd(QString pageTitle, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) override;
    virtual void pageOutputs() override;
    virtual void pageFileTransactionReceive(QString pageTitle,
                                     const QString & fileName, const util::FileTransactionInfo & transInfo,
                                     int nodeHeight) override;
    virtual void pageFileTransactionFinalize(QString pageTitle,
                                     const QString & fileName, const util::FileTransactionInfo & transInfo,
                                     int nodeHeight) override;
    virtual void pageRecieve() override;
    virtual void pageListening() override;
    virtual void pageFinalize() override;
    virtual void pageSendStarting() override;
    virtual void pageSendOnline( QString selectedAccountName, QString selectedAccountPath, qint64 amount ) override;
    virtual void pageSendSlatepack( QString selectedAccountName, QString selectedAccountPath, qint64 amount ) override;
    void pageSendFile( QString selectedAccount, qint64 amount );
    virtual void pageTransactions() override;
    // slatepack - slatepack string value to show.
    // backStateId - state ID of the caller. On 'back' will switch to this state Id
    virtual void pageShowSlatepack(QString slatepack, QString tx_uuid, int backStateId, QString txExtension, bool enableFinalize) override;

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
#ifdef FEATURE_SWAP
    virtual void pageSwapList(bool selectIncoming, bool selectOutgoing, bool selectBackup, bool selectEthWallet) override;
    virtual void pageSwapNew1() override;
    virtual void pageSwapNew2() override;
    virtual void pageSwapNew3() override;
    virtual void pageSwapEdit(QString swapId, QString stateCmd) override;
    virtual void pageSwapTradeDetails(QString swapId) override;
#endif

#ifdef FEATURE_MKTPLACE
    virtual void pageMarketplace(bool selectMyOffers, bool selectFee) override;
    virtual void pageNewUpdateOffer(QString myMsgId) override;

    virtual void showSwapBackupDlg() override;
    virtual void pageTransactionFee() override;
#endif

    virtual void pageViewHash() override;
    virtual void pageViewAccounts() override;
    virtual void pageGenerateOwnershipInput() override;
    virtual void pageGenerateOwnershipResult(const QString & proof) override;
    virtual void pageValidateOwnershipInput() override;
    virtual void pageValidateOwnershipResult(const QString & network, const QString & message, const QString & viewingKey,
                                             const QString & torAddress, const QString & mqsAddress) override;
private:
    QQmlApplicationEngine * engine = nullptr;
    QObject * mainWindow = nullptr;
    QString downloadPath = "";
};

}



#endif //MWC_QT_WALLET_MOBILEWNDMANAGER_H

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

#include "DesktopWndManager.h"
#include "control_desktop/messagebox.h"
#include "windowmanager.h"
#include "mainwindow.h"
#include "../core/global.h"
#include "../windows_desktop/a_initaccount_w.h"
#include "../windows_desktop/a_inputpassword_w.h"
#include "../windows_desktop/c_newwallet_w.h"
#include "../windows_desktop/c_newseed_w.h"
#include "../windows_desktop/c_newseedtest_w.h"
#include "../windows_desktop/c_enterseed.h"
#include "../windows_desktop/e_receive_w.h"
#include "../windows_desktop/e_outputs_w.h"
#include "../windows_desktop/e_receive_w.h"
#include "../windows_desktop/e_listening_w.h"
#include "../windows_desktop/g_finalize_w.h"
#include "../windows_desktop/g_filetransaction_w.h"
#include "../windows_desktop/g_sendStarting.h"
#include "../windows_desktop/g_sendOnline.h"
#include "../windows_desktop/g_sendOffline.h"
#include "../windows_desktop/e_transactions_w.h"
#include "../windows_desktop/h_hodl_w.h"
#include "../windows_desktop/h_hodlcold_w.h"
#include "../windows_desktop/h_hodlnode_w.h"
#include "../windows_desktop/h_hodlclaim_w.h"
#include "../dialogs_desktop/h_hodlgetsignature.h"
#include "../dialogs_desktop/g_sendconfirmationdlg.h"
#include "../windows_desktop/k_accounts_w.h"
#include "../windows_desktop/k_accounttransfer_w.h"
#include "../windows_desktop/m_airdrop_w.h"
#include "../windows_desktop/m_airdropforbtc_w.h"
#include "../windows_desktop/u_nodeinfo_w.h"
#include "../windows_desktop/w_contacts_w.h"
#include "../windows_desktop/x_events_w.h"
#include "../windows_desktop/z_progresswnd.h"
#include "../windows_desktop/x_walletconfig_w.h"
#include "../windows_desktop/x_nodeconfig_w.h"
#include "../windows_desktop/y_selectmode_w.h"
#include <QFileDialog>
#include "../core/WalletApp.h"
#include <QMessageBox>

namespace core {

using namespace control;

void DesktopWndManager::init(WindowManager * wndMan, MainWindow * mainWnd) {
    windowManager = wndMan;
    mainWindow = mainWnd;
    Q_ASSERT(windowManager);
    Q_ASSERT(mainWindow);
}


void DesktopWndManager::messageTextDlg( QString title, QString message, double widthScale) {
    if (!WalletApp::isAppCreated()) {
        // Not much what we can do. Nothing run yet. We can only exit
        return;
    }
    if (!WalletApp::isAppInitialized()) {
        // Can use System Message box only
        QMessageBox::critical(nullptr, title, message);
        return;
    }

    // Normal usage
    MessageBox::messageText(nullptr, title, message, widthScale);
}

void DesktopWndManager::messageHtmlDlg( QString title, QString message, double widthScale) {
    MessageBox::messageHTML(nullptr, title, message, widthScale);
}

// Two button box
WndManager::RETURN_CODE DesktopWndManager::questionTextDlg( QString title, QString message, QString btn1, QString btn2,
        QString btn1Tooltip, QString btn2Tooltip,
        bool default1, bool default2, double widthScale, int *ttl_blocks ) {
    if( ttl_blocks != nullptr)
    	*ttl_blocks = 1440;
    WndManager::RETURN_CODE ret = MessageBox::questionTextWithTTL(nullptr, title, message, btn1, btn2, btn1Tooltip, btn2Tooltip, default1, default2, widthScale, ttl_blocks);
    return ret;
}
WndManager::RETURN_CODE DesktopWndManager::questionHTMLDlg( QString title, QString message, QString btn1, QString btn2,
        QString btn1Tooltip, QString btn2Tooltip,
        bool default1, bool default2, double widthScale ) {
    return MessageBox::questionHTML(nullptr, title, message, btn1, btn2, btn1Tooltip, btn2Tooltip, default1, default2, widthScale);
}

WndManager::RETURN_CODE DesktopWndManager::questionTextDlg( QString title, QString message, QString btn1, QString btn2,
        QString btn1Tooltip, QString btn2Tooltip,
        bool default1, bool default2, double widthScale, QString & passwordHash, RETURN_CODE blockButton, int *ttl_blocks ) {
    if( ttl_blocks != nullptr)
        *ttl_blocks = 1440;
    WndManager::RETURN_CODE ret = MessageBox::questionTextWithTTL(nullptr, title, message, btn1, btn2, btn1Tooltip, btn2Tooltip, default1, default2, widthScale, passwordHash, blockButton, ttl_blocks);
    return ret;
}

// QFileDialog::getSaveFileName call
QString DesktopWndManager::getSaveFileName(const QString &caption, const QString &dir, const QString &filter) {
    QString fileName = QFileDialog::getSaveFileName(nullptr, caption,
                                 dir,
                                 filter);

    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return "";
    }
    return fileName;
}

// Ask for confirmation
bool DesktopWndManager::sendConfirmationDlg( QString title, QString message, double widthScale, QString passwordHash ) {
    dlg::SendConfirmationDlg confirmDlg(nullptr, title, message, widthScale, passwordHash );
    return confirmDlg.exec() == QDialog::Accepted;
}


void DesktopWndManager::pageNewWallet() {
    windowManager->switchToWindowEx( mwc::PAGE_A_NEW_WALLET,
                new wnd::NewWallet( windowManager->getInWndParent() ) );
}

void DesktopWndManager::pageInputPassword(QString pageTitle, bool lockMode) {
    windowManager->switchToWindowEx( pageTitle,
                new wnd::InputPassword( windowManager->getInWndParent(), lockMode ) );
}

void DesktopWndManager::pageInitAccount() {
    windowManager->switchToWindowEx( mwc::PAGE_A_INIT_ACCOUNT,
                new wnd::InitAccount( windowManager->getInWndParent()));
}

void DesktopWndManager::pageEnterSeed() {
    windowManager->switchToWindowEx( mwc::PAGE_A_ENTER_SEED,
                                     new wnd::EnterSeed( windowManager->getInWndParent()));
}

void DesktopWndManager::pageNewSeed(QString pageTitle, QVector<QString> seed, bool hideSubmitButton) {
    windowManager->switchToWindowEx( pageTitle,
                                    new wnd::NewSeed( windowManager->getInWndParent(),
                                                      seed, hideSubmitButton ) );
}

void DesktopWndManager::pageNewSeedTest(int wordIndex) {
    windowManager->switchToWindowEx( mwc::PAGE_A_PASSPHRASE_TEST,
            new wnd::NewSeedTest( windowManager->getInWndParent(), wordIndex ));
}

void DesktopWndManager::pageProgressWnd(QString pageTitle, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) {
    windowManager->switchToWindowEx( pageTitle,
             new wnd::ProgressWnd(windowManager->getInWndParent(), callerId, header, msgProgress, msgPlus, cancellable));
}

void DesktopWndManager::pageOutputs() {
    windowManager->switchToWindowEx(mwc::PAGE_E_OUTPUTS,
                new wnd::Outputs( windowManager->getInWndParent()));
}

void DesktopWndManager::pageFileTransaction(QString pageTitle, QString callerId,
                                            const QString & fileName, const util::FileTransactionInfo & transInfo,
                                            int nodeHeight,
                                            QString transactionType, QString processButtonName) {
    windowManager->switchToWindowEx( pageTitle,
                        new wnd::FileTransaction( windowManager->getInWndParent(), callerId,
                                fileName, transInfo, nodeHeight, transactionType, processButtonName) );
}

void DesktopWndManager::pageRecieve() {
    windowManager->switchToWindowEx( mwc::PAGE_E_RECEIVE,
               new wnd::Receive( windowManager->getInWndParent() ) );
}

void DesktopWndManager::pageListening() {

    windowManager->switchToWindowEx( mwc::PAGE_E_LISTENING,
                 new wnd::Listening( windowManager->getInWndParent()));
}

void DesktopWndManager::pageFinalize() {
    windowManager->switchToWindowEx( mwc::PAGE_G_FINALIZE_UPLOAD,
              new wnd::Finalize( windowManager->getInWndParent() ) );
}

void DesktopWndManager::pageSendStarting() {
    windowManager->switchToWindowEx( mwc::PAGE_G_SEND,
              new wnd::SendStarting( windowManager->getInWndParent()));
}

void DesktopWndManager::pageSendOnline( QString selectedAccount, int64_t amount ) {
    windowManager->switchToWindowEx( mwc::PAGE_G_SEND_ONLINE,
                                     new wnd::SendOnline( windowManager->getInWndParent(), selectedAccount, amount ));
}

void DesktopWndManager::pageSendOffline( QString selectedAccount, int64_t amount ) {
    windowManager->switchToWindowEx( mwc::PAGE_G_SEND_FILE,
                                     new wnd::SendOffline( windowManager->getInWndParent(), selectedAccount, amount ));
}

void DesktopWndManager::pageTransactions() {
    windowManager->switchToWindowEx( mwc::PAGE_E_TRANSACTION,
                      new wnd::Transactions( windowManager->getInWndParent()));
}

void DesktopWndManager::pageHodl() {
    windowManager->switchToWindowEx( mwc::PAGE_HODL,
                                     new wnd::Hodl( windowManager->getInWndParent()));
}

void DesktopWndManager::pageHodlNode() {
    windowManager->switchToWindowEx( mwc::PAGE_HODL,
                                     new wnd::HodlNode( windowManager->getInWndParent()));
}

void DesktopWndManager::pageHodlCold() {
    windowManager->switchToWindowEx( mwc::PAGE_HODL,
                                     new wnd::HodlCold( windowManager->getInWndParent()));
}

void DesktopWndManager::pageHodlClaim(QString walletHash)  {
    windowManager->switchToWindowEx( mwc::PAGE_HODL_CLAIM,
                                     new wnd::HodlClaim( windowManager->getInWndParent(), walletHash));
}

// return true(first) if press OK and input signature value(second)
QPair<bool, QString> DesktopWndManager::hodlGetSignatureDlg(QString challenge) {
    dlg::HodlGetSignature signatureDlg(nullptr, challenge);
    if ( signatureDlg.exec() == QDialog::Accepted) {
        return QPair<bool, QString>(true, signatureDlg.getSignature());
    }
    return QPair<bool, QString>(false, "");
}

void DesktopWndManager::pageAccounts() {
    windowManager->switchToWindowEx( mwc::PAGE_K_ACCOUNTS,
               new wnd::Accounts( windowManager->getInWndParent()));
}

void DesktopWndManager::pageAccountTransfer() {
    windowManager->switchToWindowEx( mwc::PAGE_K_ACCOUNT_TRANSFER,
               new wnd::AccountTransfer( windowManager->getInWndParent()));
}

void DesktopWndManager::pageAirdrop() {
    windowManager->switchToWindowEx( mwc::PAGE_M_AIRDROP,
             new wnd::Airdrop( windowManager->getInWndParent()));
}

void DesktopWndManager::pageAirdropForBTC(QString btcAddress, QString challenge, QString identifier) {
    windowManager->switchToWindowEx( mwc::PAGE_M_AIRDROP_CLAIM,
               new wnd::AirdropForBTC( windowManager->getInWndParent(), btcAddress, challenge, identifier ) );
}

void DesktopWndManager::pageNodeInfo() {
    windowManager->switchToWindowEx( mwc::PAGE_U_NODE_STATUS,
             new wnd::NodeInfo( windowManager->getInWndParent()));
}

void DesktopWndManager::pageContacts() {
    windowManager->switchToWindowEx( mwc::PAGE_W_CONTACTS,
              new wnd::Contacts( windowManager->getInWndParent()));
}

void DesktopWndManager::pageEvents() {
    windowManager->switchToWindowEx( mwc::PAGE_X_EVENTS,
              new wnd::Events( windowManager->getInWndParent()));
}

void DesktopWndManager::pageWalletConfig() {
    windowManager->switchToWindowEx( mwc::PAGE_X_WALLET_CONFIG,
              new wnd::WalletConfig( windowManager->getInWndParent()));
}

void DesktopWndManager::pageNodeConfig() {
    windowManager->switchToWindowEx( mwc::PAGE_X_WALLET_CONFIG,
             new wnd::NodeConfig( windowManager->getInWndParent()));
}

void DesktopWndManager::pageSelectMode() {
    windowManager->switchToWindowEx( mwc::PAGE_Y_SELECT_RUNNING_MODE,
              new wnd::SelectMode( windowManager->getInWndParent()));
}

void DesktopWndManager::pageWalletHome() {
    Q_ASSERT(false);
}

void DesktopWndManager::pageWalletSettings() {
    Q_ASSERT(false);
}

void DesktopWndManager::pageAccountOptions() {
    Q_ASSERT(false);
}

}

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
#include "../windows_desktop/a_initfirsttime_w.h"
#include "../windows_desktop/a_initaccount_w.h"
#include "../windows_desktop/a_inputpassword_w.h"
#include "../windows_desktop/c_newseed_w.h"
#include "../windows_desktop/c_newseedtest_w.h"
#include "../windows_desktop/c_enterseed.h"
#include "../windows_desktop/e_receive_w.h"
#include "../windows_desktop/e_outputs_w.h"
#include "../windows_desktop/e_listening_w.h"
#include "../windows_desktop/g_finalize_w.h"
#include "../windows_desktop/g_filetransaction_finalize_w.h"
#include "../windows_desktop/g_filetransaction_receive_w.h"
#include "../windows_desktop/g_sendStarting.h"
#include "../windows_desktop/g_sendOnline.h"
#include "../windows_desktop/g_sendOffline.h"
#include "../windows_desktop/g_resultedslatepack_w.h"
#include "../windows_desktop/e_transactions_w.h"
#include "../dialogs_desktop/g_sendconfirmationdlg.h"
#include "../windows_desktop/k_accounts_w.h"
#include "../windows_desktop/k_accounttransfer_w.h"
#include "../windows_desktop/u_nodeinfo_w.h"
#include "../windows_desktop/w_contacts_w.h"
#include "../windows_desktop/x_events_w.h"
#include "../windows_desktop/z_progresswnd.h"
#include "../windows_desktop/x_walletconfig_w.h"
#include "../windows_desktop/x_nodeconfig_w.h"
#include "../windows_desktop/y_selectmode_w.h"
#include "../windows_desktop/s_swaplist_w.h"
#include "../windows_desktop/s_editswap_w.h"
#include "../windows_desktop/s_newswap1_w.h"
#include "../windows_desktop/s_newswap2_w.h"
#include "../windows_desktop/s_newswap3_w.h"
#include "../windows_desktop/s_tradedetails_w.h"
#include "../windows_desktop/s_mrktswlist_w.h"
#include "../windows_desktop/s_mrktswapnew_w.h"
#include <QFileDialog>
#include "../core/WalletApp.h"
#include <QMessageBox>
#include "../util_desktop/timeoutlock.h"
#include "../util/Process.h"
#include "../dialogs_desktop/showwalletstoppingmessagedlg.h"
#include "../dialogs_desktop/s_swapbackupdlg.h"
#include "../state/state.h"
#include "../state/u_nodeinfo.h"

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

// QFileDialog::getSaveFileName call
QString DesktopWndManager::getOpenFileName(const QString &caption, const QString &dir, const QString &filter) {
    QString fileName = QFileDialog::getOpenFileName(nullptr, caption,
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

static dlg::ShowWalletStoppingMessageDlg * showWalletMessageDlg = nullptr;

// Stopping wallet message
void DesktopWndManager::showWalletStoppingMessage(int taskTimeout) {
    if (showWalletMessageDlg == nullptr) {
        showWalletMessageDlg = new dlg::ShowWalletStoppingMessageDlg(nullptr, taskTimeout);
    }
    showWalletMessageDlg->show();
}

void DesktopWndManager::hideWalletStoppingMessage() {
    if (showWalletMessageDlg != nullptr) {
        showWalletMessageDlg->hide();
        showWalletMessageDlg->deleteLater();
        showWalletMessageDlg = nullptr;
    }
}


void DesktopWndManager::pageInitFirstTime() {
    windowManager->switchToWindowEx( mwc::PAGE_A_FIRST_TIME,
                                     new wnd::InitFirstTime( windowManager->getInWndParent() ) );
}

void DesktopWndManager::pageInputPassword(QString pageTitle, bool lockMode) {
    windowManager->switchToWindowEx( pageTitle,
                new wnd::InputPassword( windowManager->getInWndParent(), lockMode ) );
}

void DesktopWndManager::pageInitAccount(QString path, bool restoredFromSeed) {
    windowManager->switchToWindowEx( mwc::PAGE_A_INIT_ACCOUNT,
                new wnd::InitAccount( windowManager->getInWndParent(), path, restoredFromSeed ));
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

void DesktopWndManager::pageFileTransactionReceive(QString pageTitle,
                                            const QString & fileNameOrSlatepack,
                                            const util::FileTransactionInfo & transInfo,
                                            int nodeHeight) {
    windowManager->switchToWindowEx( pageTitle,
                        new wnd::FileTransactionReceive( windowManager->getInWndParent(),
                                                  fileNameOrSlatepack, transInfo, nodeHeight) );
}

void DesktopWndManager::pageFileTransactionFinalize(QString pageTitle,
                                            const QString & fileNameOrSlatepack,
                                            const util::FileTransactionInfo & transInfo,
                                            int nodeHeight) {
    windowManager->switchToWindowEx( pageTitle,
                                     new wnd::FileTransactionFinalize( windowManager->getInWndParent(),
                                                               fileNameOrSlatepack, transInfo, nodeHeight) );
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

void DesktopWndManager::pageSendFile( QString selectedAccount, int64_t amount ) {
    windowManager->switchToWindowEx( mwc::PAGE_G_SEND_FILE,
                                     new wnd::SendOffline( windowManager->getInWndParent(), selectedAccount, amount, false ));
}

void DesktopWndManager::pageSendSlatepack( QString selectedAccount, int64_t amount ) {
    windowManager->switchToWindowEx( mwc::PAGE_G_SEND_SLATEPACK,
                                     new wnd::SendOffline( windowManager->getInWndParent(), selectedAccount, amount, true ));
}

void DesktopWndManager::pageTransactions() {
    windowManager->switchToWindowEx( mwc::PAGE_E_TRANSACTION,
                      new wnd::Transactions( windowManager->getInWndParent()));
}

// slatepack - slatepack string value to show.
// backStateId - state ID of the caller. On 'back' will switch to this state Id
void DesktopWndManager::pageShowSlatepack(QString slatepack, int backStateId, QString txExtension, bool enableFinalize) {
    windowManager->switchToWindowEx( mwc::PAGE_G_SLATEPACK,
                                     new wnd::ResultedSlatepack( windowManager->getInWndParent(), slatepack, backStateId, txExtension, enableFinalize ));
}

void DesktopWndManager::pageAccounts() {
    windowManager->switchToWindowEx( mwc::PAGE_K_ACCOUNTS,
               new wnd::Accounts( windowManager->getInWndParent()));
}

void DesktopWndManager::pageAccountTransfer() {
    windowManager->switchToWindowEx( mwc::PAGE_K_ACCOUNT_TRANSFER,
               new wnd::AccountTransfer( windowManager->getInWndParent()));
}

void DesktopWndManager::pageNodeInfo() {
    state::NodeInfo * ni =  (state::NodeInfo *)state::getState(state::STATE::NODE_INFO);
    Q_ASSERT(ni);

    wallet::MwcNodeConnection connectionInfo = ni->getNodeConnection();
    QString nodeType;
    switch(connectionInfo.connectionType) {
        case wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD:
            nodeType = "Cloud";
            break;
        case wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CUSTOM:
            nodeType = "Custom";
            break;
        case wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL:
            nodeType = "Embedded";
            break;
        default:
            Q_ASSERT(false);
    }

    windowManager->switchToWindowEx( mwc::PAGE_U_NODE_STATUS + " (" + nodeType + ")",
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
    windowManager->switchToWindowEx( mwc::PAGE_X_NODE_CONFIG,
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

void DesktopWndManager::pageSwapList() {
    windowManager->switchToWindowEx( mwc::PAGE_S_SWAP_LIST,
        new wnd::SwapList( windowManager->getInWndParent()));
}

void DesktopWndManager::pageSwapNew1() {
    windowManager->switchToWindowEx( mwc::PAGE_S_SWAP_NEW + " 1/3",
                                     new wnd::NewSwap1( windowManager->getInWndParent()));
}

void DesktopWndManager::pageSwapNew2() {
    windowManager->switchToWindowEx( mwc::PAGE_S_SWAP_NEW + " 2/3",
                                     new wnd::NewSwap2( windowManager->getInWndParent()));
}
void DesktopWndManager::pageSwapNew3() {
    windowManager->switchToWindowEx( mwc::PAGE_S_SWAP_NEW + " 3/3",
                                     new wnd::NewSwap3( windowManager->getInWndParent()));
}

void DesktopWndManager::pageSwapEdit(QString swapId, QString stateCmd) {
    windowManager->switchToWindowEx( "",
         new wnd::EditSwap( windowManager->getInWndParent(), swapId, stateCmd));
}
void DesktopWndManager::pageSwapTradeDetails(QString swapId) {
    windowManager->switchToWindowEx( "",
         new wnd::TradeDetails( windowManager->getInWndParent(), swapId));
}

void DesktopWndManager::showBackupDlg(QString swapId, int backupId) {
    util::TimeoutLockObject to( "SwapBackupDlg" );
    dlg::SwapBackupDlg  backup(nullptr, swapId, backupId);
    backup.exec();
}

// Swap marketplace
void DesktopWndManager::pageMarketplace(bool selectMyOffers) {
    windowManager->switchToWindowEx( mwc::PAGE_S_MKT_LIST,
                                     new wnd::MrktSwList( windowManager->getInWndParent(), selectMyOffers));
}

void DesktopWndManager::pageNewUpdateOffer(QString myMsgId) {
    windowManager->switchToWindowEx( mwc::PAGE_S_MKT_NEW_OFFER,
                                     new wnd::MrktSwapNew( windowManager->getInWndParent(), myMsgId));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Select a directory that has a wallet data
QString selectWalletDirectory() {
    util::TimeoutLockObject to( "WalletConfig" );

    QPair<bool,QString> basePath = ioutils::getAppDataPath();
    if (!basePath.first) {
        control::MessageBox::messageText(nullptr, "Error", basePath.second);
        return "";
    }

    QString dir = QFileDialog::getExistingDirectory(
            nullptr,
            "Select your wallet folder name",
            basePath.second);
    if (dir.isEmpty())
        return "";
    auto dirOk = util::validateMwc713Str(dir);
    if (!dirOk.first) {
        core::getWndManager()->messageTextDlg("Directory Name",
                                              "This directory name is not acceptable.\n" + dirOk.second);
        return "";
    }

    QDir baseDir(basePath.second);
    QString walletDir = baseDir.relativeFilePath(dir);

    QVector<QString>  networkArch = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(walletDir, state::getStateContext()->appContext ); // local path as writen in config
    QString runningArc = util::getBuildArch();

    // Just in case. Normally will never be called
    if ( runningArc != networkArch[1] ) {
        control::MessageBox::messageText(nullptr, "Wallet data architecture mismatch",
                                         "Your mwc713 seed at '"+ walletDir +"' was created with "+ networkArch[1] +" bits version of the wallet. You are using " + runningArc + " bit version.");
        return "";
    }

    return walletDir;
}



}

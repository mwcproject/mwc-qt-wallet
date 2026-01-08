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
#include "../windows_desktop/a_seedlength.h"
#include "../windows_desktop/a_inputpassword_w.h"
#include "../windows_desktop/c_newseed_w.h"
#include "../windows_desktop/c_newseedtest_w.h"
#include "../windows_desktop/c_enterseed.h"
#include "../windows_desktop/e_receive_w.h"
#include "../windows_desktop/e_receiveSpOnly_w.h"
#include "../windows_desktop/e_outputs_w.h"
#include "../windows_desktop/e_listening_w.h"
#include "../windows_desktop/g_finalize_w.h"
#include "../windows_desktop/g_filetransaction_finalize_w.h"
#include "../windows_desktop/g_filetransaction_receive_w.h"
#include "../windows_desktop/g_sendStarting.h"
#include "../windows_desktop/g_sendOnline.h"
#include "../windows_desktop/g_sendOffline.h"
#include "../windows_desktop/g_sendOnlineOnly.h"
#include "../windows_desktop/g_sendOfflineOnly.h"
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
#include "../windows_desktop/s_mktinttx_w.h"
#include "../windows_desktop/v_viewhash_w.h"
#include "../windows_desktop/v_viewoutputs_w.h"
#include "../windows_desktop/v_generateownershipproofinput_w.h"
#include "../windows_desktop/v_generateownershipproofresult_w.h"
#include "../windows_desktop/v_validateownershipproofinput_w.h"
#include "../windows_desktop/v_validateownershipproofresult_w.h"
#include <QFileDialog>
#include "../core/WalletApp.h"
#include <QMessageBox>
#include "../util_desktop/timeoutlock.h"
#include "../util/Process.h"
#include "../dialogs_desktop/s_swapbackupdlg.h"
#include "../state/state.h"
#include "../state/u_nodeinfo.h"
#include "../dialogs_desktop/g_sendconfirmationspdlg.h"
#include "util/ioutils.h"
#include "util/Log.h"

namespace core {

using namespace control;

void DesktopWndManager::init(WindowManager * wndMan, MainWindow * mainWnd) {
    windowManager = wndMan;
    mainWindow = mainWnd;
    Q_ASSERT(windowManager);
    Q_ASSERT(mainWindow);
}

// Current active non model window. Will be used as a parent for dialogs
QWidget * DesktopWndManager::getCurrentWnd() const {
    if (windowManager==nullptr)
        return nullptr;

    return windowManager->getCurrentWnd();
}


void DesktopWndManager::messageTextDlg( QString title, QString message, double widthScale) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::messageTextDlg with title=" + title + " message=" + message + " widthScale=" + QString::number(widthScale));
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
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::messageHtmlDlg with title=" + title + " message=" + message + " widthScale=" + QString::number(widthScale));
    MessageBox::messageHTML(nullptr, title, message, widthScale);
}

// Two button box
WndManager::RETURN_CODE DesktopWndManager::questionTextDlg( QString title, QString message, QString btn1, QString btn2,
        QString btn1Tooltip, QString btn2Tooltip,
        bool default1, bool default2, double widthScale ) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::questionTextDlg with title=" + title + " message=" + message + " btn1=" + btn1 + " btn2=" + btn2 + " widthScale=" + QString::number(widthScale));
    WndManager::RETURN_CODE ret = MessageBox::questionText(nullptr, title, message, btn1, btn2, btn1Tooltip, btn2Tooltip, default1, default2, widthScale );
    return ret;
}
WndManager::RETURN_CODE DesktopWndManager::questionHTMLDlg( QString title, QString message, QString btn1, QString btn2,
        QString btn1Tooltip, QString btn2Tooltip,
        bool default1, bool default2, double widthScale ) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::questionHTMLDlg with title=" + title + " message=" + message + " btn1=" + btn1 + " btn2=" + btn2 + " widthScale=" + QString::number(widthScale));
    return MessageBox::questionHTML(nullptr, title, message, btn1, btn2, btn1Tooltip, btn2Tooltip, default1, default2, widthScale);
}

// QFileDialog::getSaveFileName call
QString DesktopWndManager::getSaveFileName(const QString &caption, const QString &dir, const QString &filter) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::getSaveFileName with caption=" + caption + " dir=" + dir + " filter=" + filter);

    QFileDialog dialog(getCurrentWnd(), caption, dir, filter);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (dialog.exec() == QDialog::Accepted) {
        QString fileName = dialog.selectedFiles().first();
        auto fileOk = util::validateMwc713Str(fileName);
        if (!fileOk.first) {
            core::getWndManager()->messageTextDlg("File Path",
                                                  "This file path is not acceptable.\n" + fileOk.second);
            return "";
        }
        return fileName;
    }

    return "";
}

// QFileDialog::getSaveFileName call
QString DesktopWndManager::getOpenFileName(const QString &caption, const QString &dir, const QString &filter) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::getOpenFileName with caption=" + caption + " dir=" + dir + " filter=" + filter);

    // Note, we can't use native dialogs. Under macOS it doesn't work well, it is closed if some events happens.
    QFileDialog dlg(getCurrentWnd(), caption, dir, filter);
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    dlg.setOption(QFileDialog::ReadOnly, true);
    dlg.setFileMode(QFileDialog::ExistingFile);

    if (dlg.exec() == QDialog::Accepted) {
        QString fileName = dlg.selectedFiles().value(0);
        auto fileOk = util::validateMwc713Str(fileName);
        if (!fileOk.first) {
            core::getWndManager()->messageTextDlg("File Path",
                                                  "This file path is not acceptable.\n" + fileOk.second);
            return "";
        }
        return fileName;
    }
    return "";
}

bool DesktopWndManager::sendConfirmationSlatepackDlg( QString title, QString messageBody, double widthScale, int inputsNum, int * ttl ) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::sendConfirmationSlatepackDlg with title=" + title +
        " messageBody=" + messageBody + " widthScale=" + QString::number(widthScale) + " inputsNum=" + QString::number(inputsNum));
    dlg::SendConfirmationSlatePackDlg confirmDlg(nullptr, title, messageBody, widthScale, inputsNum, *ttl);
    if (confirmDlg.exec() == QDialog::Accepted) {
        *ttl = confirmDlg.getTTlBlocks();
        return true;
    }
    return false;
}


// Ask for confirmation
bool DesktopWndManager::sendConfirmationDlg( QString title, QString message, double widthScale, int inputsNum ) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::sendConfirmationDlg with title=" + title +
        " message=" + message + " widthScale=" + QString::number(widthScale) + " inputsNum=" + QString::number(inputsNum));
    dlg::SendConfirmationDlg confirmDlg(nullptr, title, message, widthScale, inputsNum );
    return confirmDlg.exec() == QDialog::Accepted;
}

void DesktopWndManager::pageInitFirstTime() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageInitFirstTime");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_A_FIRST_TIME,
                                     new wnd::InitFirstTime( windowManager->getInWndParent() ) );
}

void DesktopWndManager::pageInputPassword(QString pageTitle, bool lockMode) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageInputPassword with pageTitle=" + pageTitle +
        " lockMode=" + QString(lockMode ? "true" : "false"));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( pageTitle,
                new wnd::InputPassword( windowManager->getInWndParent(), lockMode ) );
}

void DesktopWndManager::pageInitAccount(QString path, bool restoredFromSeed) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageInitAccount with path=" + path +
        " restoredFromSeed=" + QString(restoredFromSeed ? "true" : "false"));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_A_INIT_ACCOUNT,
                new wnd::InitAccount( windowManager->getInWndParent(), path, restoredFromSeed ));
}

void DesktopWndManager::pageSeedLength() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSeedLength");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_A_SEED_LENGTH,
                                     new wnd::SeedLength( windowManager->getInWndParent()));
}


void DesktopWndManager::pageEnterSeed() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageEnterSeed");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_A_ENTER_SEED,
                                     new wnd::EnterSeed( windowManager->getInWndParent()));
}

void DesktopWndManager::pageNewSeed(QString pageTitle, QVector<QString> seed, bool hideSubmitButton) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageNewSeed with pageTitle=" + pageTitle +
        " seedLength=" + QString::number(seed.size()) + " hideSubmitButton=" + QString(hideSubmitButton ? "true" : "false"));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( pageTitle,
                                    new wnd::NewSeed( windowManager->getInWndParent(),
                                                      seed, hideSubmitButton ) );
}

void DesktopWndManager::pageNewSeedTest(int wordIndex) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageNewSeedTest with wordIndex=" + QString::number(wordIndex));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_A_PASSPHRASE_TEST,
            new wnd::NewSeedTest( windowManager->getInWndParent(), wordIndex ));
}

void DesktopWndManager::pageProgressWnd(QString pageTitle, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageProgressWnd with pageTitle=" + pageTitle +
        " callerId=" + callerId + " header=" + header + " msgProgress=" + msgProgress + " msgPlus=" + msgPlus + " cancellable=" + QString(cancellable ? "true" : "false"));
    restoreLeftBarShownStatus();

    if (mainWindow->isLeftBarShown()) {
        isLeftBarShownStatus = 2;
        mainWindow->updateLeftBar(false);
    }

    windowManager->switchToWindowEx( pageTitle,
             new wnd::ProgressWnd(windowManager->getInWndParent(), callerId, header, msgProgress, msgPlus, cancellable));
}

void DesktopWndManager::pageOutputs() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageOutputs");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx(mwc::PAGE_E_OUTPUTS,
                new wnd::Outputs( windowManager->getInWndParent()));
}

void DesktopWndManager::pageFileTransactionReceive(QString pageTitle,
                                            const QString & fileNameOrSlatepack,
                                            const util::FileTransactionInfo & transInfo,
                                            int nodeHeight) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageFileTransactionReceive with pageTitle=" +
        pageTitle + " fileNameOrSlatepack=<hidden> nodeHeight=" + QString::number(nodeHeight));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( pageTitle,
                        new wnd::FileTransactionReceive( windowManager->getInWndParent(),
                                                  fileNameOrSlatepack, transInfo, nodeHeight) );
}

void DesktopWndManager::pageFileTransactionFinalize(QString pageTitle,
                                            const QString & fileNameOrSlatepack,
                                            const util::FileTransactionInfo & transInfo,
                                            int nodeHeight) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageFileTransactionFinalize with pageTitle=" + pageTitle + " fileNameOrSlatepack=<hidden> nodeHeight=" + QString::number(nodeHeight));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( pageTitle,
                                     new wnd::FileTransactionFinalize( windowManager->getInWndParent(),
                                                               fileNameOrSlatepack, transInfo, nodeHeight) );
}

void DesktopWndManager::pageRecieve() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageRecieve");
    restoreLeftBarShownStatus();
    if (state::getStateContext()->appContext->isFeatureMWCMQS() || state::getStateContext()->appContext->isFeatureTor()) {
        // Receive UI for many
        windowManager->switchToWindowEx( mwc::PAGE_E_RECEIVE,
               new wnd::Receive( windowManager->getInWndParent() ) );
    }
    else {
        // UI for SP only
        windowManager->switchToWindowEx( mwc::PAGE_E_RECEIVE,
                new wnd::ReceiveSpOnly( windowManager->getInWndParent() ) );

    }
}

void DesktopWndManager::pageListening() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageListening");
    restoreLeftBarShownStatus();
    QString pageTitle = state::getStateContext()->appContext->isFeatureMWCMQS() || state::getStateContext()->appContext->isFeatureTor() ?
                    mwc::PAGE_E_LISTENING_ONLINE_LISTERS : mwc::PAGE_E_LISTENING_SLATEPACK_ONLY;
    windowManager->switchToWindowEx( pageTitle,
                 new wnd::Listening( windowManager->getInWndParent()));
}

void DesktopWndManager::pageFinalize() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageFinalize");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_G_FINALIZE_UPLOAD,
              new wnd::Finalize( windowManager->getInWndParent() ) );
}

void DesktopWndManager::pageSendStarting() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSendStarting");
    restoreLeftBarShownStatus();

    core::AppContext * context = state::getStateContext()->appContext;

    bool hasSlatepack = context->isFeatureSlatepack();
    bool hasOnline = context->isFeatureMWCMQS() || context->isFeatureTor();

    Q_ASSERT(hasSlatepack || hasOnline);

    if (hasSlatepack && hasOnline) {
        windowManager->switchToWindowEx( mwc::PAGE_G_SEND,
                  new wnd::SendStarting( windowManager->getInWndParent()));
    }
    else if (hasOnline) {
        windowManager->switchToWindowEx( mwc::PAGE_G_SEND_ONLINE,
                  new wnd::SendOnlineOnly( windowManager->getInWndParent()));
    }
    else if (hasSlatepack) {
        windowManager->switchToWindowEx( mwc::PAGE_G_SEND_SLATEPACK,
                  new wnd::SendOfflineOnly( windowManager->getInWndParent()));
    }
    else {
        Q_ASSERT(false);
    }
}

void DesktopWndManager::pageSendOnline( QString selectedAccountName, QString selectedAccountPath, int64_t amount ) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSendOnline with selectedAccount=" + selectedAccountName + " amount=" + QString::number(amount));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_G_SEND_ONLINE,
                                     new wnd::SendOnline( windowManager->getInWndParent(), selectedAccountName, selectedAccountPath, amount ));
}

void DesktopWndManager::pageSendSlatepack( QString selectedAccountName, QString selectedAccountPath, int64_t amount ) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSendSlatepack with selectedAccount=" + selectedAccountName + " amount=" + QString::number(amount));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_G_SEND_SLATEPACK,
                                     new wnd::SendOffline( windowManager->getInWndParent(), selectedAccountName, selectedAccountPath, amount ));
}

void DesktopWndManager::pageTransactions() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageTransactions");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_E_TRANSACTION,
                      new wnd::Transactions( windowManager->getInWndParent()));
}

// slatepack - slatepack string value to show.
// backStateId - state ID of the caller. On 'back' will switch to this state Id
void DesktopWndManager::pageShowSlatepack(QString slatepack, QString tx_uuid, int backStateId, QString txExtension, bool enableFinalize) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageShowSlatepack with <slatepack> backStateId=" + QString::number(backStateId) +
        " txExtension=" + txExtension + " enableFinalize=" + QString(enableFinalize ? "true" : "false"));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_G_SLATEPACK,
                                     new wnd::ResultedSlatepack( windowManager->getInWndParent(), slatepack, tx_uuid, backStateId, txExtension, enableFinalize ));
}

void DesktopWndManager::pageAccounts() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageAccounts");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_K_ACCOUNTS,
               new wnd::Accounts( windowManager->getInWndParent()));
}

void DesktopWndManager::pageAccountTransfer() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageAccountTransfer");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_K_ACCOUNT_TRANSFER,
               new wnd::AccountTransfer( windowManager->getInWndParent()));
}

void DesktopWndManager::pageNodeInfo() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageNodeInfo");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_U_NODE_STATUS,
             new wnd::NodeInfo( windowManager->getInWndParent()));
}

void DesktopWndManager::pageContacts() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageContacts");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_W_CONTACTS,
              new wnd::Contacts( windowManager->getInWndParent()));
}

void DesktopWndManager::pageEvents() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageEvents");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_X_EVENTS,
              new wnd::Events( windowManager->getInWndParent()));
}

void DesktopWndManager::pageWalletConfig() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageWalletConfig");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_X_WALLET_CONFIG,
              new wnd::WalletConfig( windowManager->getInWndParent()));
}

void DesktopWndManager::pageNodeConfig() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageNodeConfig");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_X_NODE_CONFIG,
             new wnd::NodeConfig( windowManager->getInWndParent()));
}

void DesktopWndManager::pageSelectMode() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSelectMode");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_Y_SELECT_RUNNING_MODE,
              new wnd::SelectMode( windowManager->getInWndParent()));
}

void DesktopWndManager::pageWalletHome() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageWalletHome");
    Q_ASSERT(false);
}

void DesktopWndManager::pageWalletSettings() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageWalletSettings");
    Q_ASSERT(false);
}

void DesktopWndManager::pageAccountOptions() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageAccountOptions");
    Q_ASSERT(false);
}

#ifdef FEATURE_SWAP
void DesktopWndManager::pageSwapList(bool selectIncoming, bool selectOutgoing, bool selectBackup, bool selectEthWallet) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSwapList with selectIncoming=" + QString(selectIncoming ? "true" : "false") +
        " selectOutgoing=" + QString(selectOutgoing ? "true" : "false") + " selectBackup=" + QString(selectBackup ? "true" : "false") +
        " selectEthWallet=" + QString(selectEthWallet ? "true" : "false"));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_S_SWAP_LIST,
        new wnd::SwapList( windowManager->getInWndParent(), selectIncoming, selectOutgoing, selectBackup, selectEthWallet));
}

void DesktopWndManager::pageSwapNew1() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSwapNew1");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_S_SWAP_NEW + " 1/3",
                                     new wnd::NewSwap1( windowManager->getInWndParent()));
}

void DesktopWndManager::pageSwapNew2() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSwapNew2");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_S_SWAP_NEW + " 2/3",
                                     new wnd::NewSwap2( windowManager->getInWndParent()));
}
void DesktopWndManager::pageSwapNew3() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSwapNew3");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_S_SWAP_NEW + " 3/3",
                                     new wnd::NewSwap3( windowManager->getInWndParent()));
}

void DesktopWndManager::pageSwapEdit(QString swapId, QString stateCmd) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSwapEdit with swapId=" + swapId + " stateCmd=" + stateCmd);
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( "",
         new wnd::EditSwap( windowManager->getInWndParent(), swapId, stateCmd));
}
void DesktopWndManager::pageSwapTradeDetails(QString swapId) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageSwapTradeDetails with swapId=" + swapId);
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( "",
         new wnd::TradeDetails( windowManager->getInWndParent(), swapId));
}
#endif

#ifdef FEATURE_MKTPLACE
// Swap marketplace
void DesktopWndManager::pageMarketplace(bool selectMyOffers, bool selectFee) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageMarketplace with selectMyOffers=" + QString(selectMyOffers ? "true" : "false") +
        " selectFee=" + QString(selectFee ? "true" : "false"));
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_S_MKT_LIST,
                                     new wnd::MrktSwList( windowManager->getInWndParent(), selectMyOffers, selectFee));
}

void DesktopWndManager::pageNewUpdateOffer(QString myMsgId) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageNewUpdateOffer with myMsgId=" + myMsgId);
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( myMsgId.isEmpty() ? mwc::PAGE_S_MKT_NEW_OFFER : mwc::PAGE_S_MKT_OFFER_UPDATE,
                                     new wnd::MrktSwapNew( windowManager->getInWndParent(), myMsgId));
}

void DesktopWndManager::showSwapBackupDlg() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::showSwapBackupDlg");
    dlg::SwapBackupDlg dlg(nullptr);
    dlg.exec();
}

void DesktopWndManager::pageTransactionFee() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageTransactionFee");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_S_MKT_TX_FEE,
                                     new wnd::IntegrityTransactions( windowManager->getInWndParent()));
}
#endif

void DesktopWndManager::pageViewHash()  {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageViewHash");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_V_VIEW_HASH,
                                     new wnd::ViewHash( windowManager->getInWndParent()));
}

void DesktopWndManager::pageViewAccounts() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageViewAccounts");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_V_VIEW_OUTPUTS,
                                     new wnd::ViewOutputs(windowManager->getInWndParent()));
}

void DesktopWndManager::pageGenerateOwnershipInput() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageGenerateOwnershipInput");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_V_GEN_OWNERSHIP_PROOF,
                                     new wnd::GenerateOwnershipProofInput(windowManager->getInWndParent()));
}

void DesktopWndManager::pageGenerateOwnershipResult(const QString & proof) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageGenerateOwnershipResult with proof=<hidden>");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_V_GEN_OWNERSHIP_PROOF,
                                     new wnd::GenerateOwnershipProofResult(windowManager->getInWndParent(), proof));
}

void DesktopWndManager::pageValidateOwnershipInput() {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageValidateOwnershipInput");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_V_VALID_OWNERSHIP_PROOF,
                                     new wnd::ValidateOwnershipProofInput(windowManager->getInWndParent()));
}

void DesktopWndManager::pageValidateOwnershipResult(const QString & network, const QString & message, const QString & viewingKey,
                        const QString & torAddress, const QString & mqsAddress) {
    logger::logInfo(logger::QT_WALLET, "Call DesktopWndManager::pageValidateOwnershipResult with network=" + network +
        " message=<hidden> viewingKey=<hidden> torAddress=<hidden> mqsAddress=<hidden>");
    restoreLeftBarShownStatus();
    windowManager->switchToWindowEx( mwc::PAGE_V_VALID_OWNERSHIP_PROOF,
                                     new wnd::ValidateOwnershipProofResult(windowManager->getInWndParent(), network, message, viewingKey, torAddress, mqsAddress));
}

void DesktopWndManager::restoreLeftBarShownStatus() {
    if (isLeftBarShownStatus==0)
        return;

    if (isLeftBarShownStatus==1 && mainWindow)
        mainWindow->updateLeftBar(false);

    if (isLeftBarShownStatus==2 && mainWindow)
        mainWindow->updateLeftBar(true);

    isLeftBarShownStatus = 0;
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
                                         "Your wallet seed at '"+ walletDir +"' was created with a "+ networkArch[1] +"-bit version of the wallet. You are using a " + runningArc + "-bit version.");
        return "";
    }

    return walletDir;
}



}

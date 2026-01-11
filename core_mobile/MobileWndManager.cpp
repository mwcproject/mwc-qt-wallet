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

#include "MobileWndManager.h"
#include <QCoreApplication>
#include <QThread>
#include <QQmlApplicationEngine>
#include <QMessageBox>
#include "../state/state.h"
#include "../util/Log.h"
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace core {

// NOTE: all  xxxxDlg methids must be modal. In mobile conetext it is mean that we should wait for closure and return back the result.
void MobileWndManager::init(QQmlApplicationEngine * _engine) {
    engine = _engine;
    Q_ASSERT(engine);

    engine->load(QUrl(QStringLiteral("qrc:/windows_mobile/main.qml")));
    mainWindow = engine->rootObjects().first();
    mainWindow->setProperty("currentState", 0);
    QStringList downloadPaths = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
    for (int i = 0; i < downloadPaths.length(); i++) {
        QFileInfo fileInfo(downloadPaths[i]);
        if (fileInfo.isWritable()) {
            downloadPath = downloadPaths[i];
            mainWindow->setProperty("downloadPath", downloadPath);
            break;
        }
    }
}


void MobileWndManager::messageTextDlg( QString title, QString message, double widthScale) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::messageTextDlg with title=" + title +
        " message=" + message + " widthScale=" + QString::number(widthScale));
    if (engine== nullptr) {
        Q_ASSERT(false); // early crash, not much what we can do. May be do some logs (message is an error description)?
        return;
    }
    Q_UNUSED(widthScale)
    if (mainWindow) {
        QVariant retValue;
        QMetaObject::invokeMethod(mainWindow, "openMessageTextDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, message));
    } else {
        QMessageBox::critical(nullptr, title,message);
    }
}

void MobileWndManager::messageHtmlDlg( QString title, QString message, double widthScale)  {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::messageHtmlDlg with title=" + title +
        " message=" + message + " widthScale=" + QString::number(widthScale));
    Q_UNUSED(widthScale)

    QVariant retValue;
    QString msg = message.replace("<b>", "").replace("</b>", "").replace("<br>", "\n");
    QMetaObject::invokeMethod(mainWindow, "openMessageTextDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, msg));
}

// Two button box
WndManager::RETURN_CODE MobileWndManager::questionTextDlg( QString title, QString message, QString btn1, QString btn2,
                                                           QString btn1Tooltip, QString btn2Tooltip,
                                                           bool default1, bool default2, double widthScale, int *ttl_blocks) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::questionTextDlg with title=" + title +
        " message=" + message + " btn1=" + btn1 + " btn2=" + btn2);
    Q_UNUSED(btn1Tooltip) // Mobile doesn't have any tooltips
    Q_UNUSED(btn2Tooltip)
    Q_UNUSED(widthScale)
    Q_UNUSED(default1)
    Q_UNUSED(default2)

    if(mainWindow) {
        mainWindow->setProperty("questionTextDlgResponse", -1);
        QVariant retValue;
        QMetaObject::invokeMethod(mainWindow, "openQuestionTextDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, message), Q_ARG(QVariant, btn1), Q_ARG(QVariant, btn2), Q_ARG(QVariant, ""), Q_ARG(QVariant, 0), Q_ARG(QVariant, *ttl_blocks));
        while(mainWindow->property("questionTextDlgResponse") == -1) {
            QCoreApplication::processEvents();
            QThread::usleep(50);
        }
        if (mainWindow->property("questionTextDlgResponse") == 1) {
//            if (ttl_blocks != nullptr) {
//                *ttl_blocks = mainWindow->property("ttl_blocks").toInt();
//            }
            return WndManager::RETURN_CODE::BTN2;
        } else
            return WndManager::RETURN_CODE::BTN1;
    } else {
        if ( QMessageBox::Yes == QMessageBox::question(nullptr, title, message) )
            return WndManager::RETURN_CODE::BTN2;
        else
            return WndManager::RETURN_CODE::BTN1;
    }
}

WndManager::RETURN_CODE MobileWndManager::questionHTMLDlg( QString title, QString message, QString btn1, QString btn2,
                                                           QString btn1Tooltip, QString btn2Tooltip,
                                                           bool default1, bool default2, double widthScale )  {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::questionHTMLDlg with title=" + title +
        " message=" + message + " btn1=" + btn1 + " btn2=" + btn2);
    Q_UNUSED(btn1Tooltip) // Mobile doesn't have any tooltips
    Q_UNUSED(btn2Tooltip)
    Q_UNUSED(title)
    Q_UNUSED(message)
    Q_UNUSED(widthScale)
    Q_UNUSED(default1)
    Q_UNUSED(default2)
    Q_UNUSED(btn1)
    Q_UNUSED(btn2)

    Q_ASSERT(false); // implement me
    return WndManager::RETURN_CODE::BTN1;
}

// Password accepted as a HASH. EMpty String mean that no password is set.
// After return, passwordHash value will have input raw Password value. So it can be user for wallet
WndManager::RETURN_CODE MobileWndManager::questionTextDlg( QString title, QString message, QString btn1, QString btn2,
                                                           QString btn1Tooltip, QString btn2Tooltip,
                                                           bool default1, bool default2, double widthScale, QString & passwordHash, WndManager::RETURN_CODE blockButton, int *ttl_blocks)  {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::questionTextDlg with title=" + title +
        " message=" + message + " btn1=" + btn1 + " btn2=" + btn2 + " passwordHash=<hidden>");
    Q_UNUSED(btn1Tooltip) // Mobile doesn't have any tooltips
    Q_UNUSED(btn2Tooltip)
    Q_UNUSED(default1)
    Q_UNUSED(default2)
    Q_UNUSED(widthScale)

    mainWindow->setProperty("questionTextDlgResponse", -1);
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "openQuestionTextDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, message), Q_ARG(QVariant, btn1), Q_ARG(QVariant, btn2), Q_ARG(QVariant, blockButton == WndManager::RETURN_CODE::BTN1 ? 0 : 1), Q_ARG(QVariant, *ttl_blocks), Q_ARG(QVariant, passwordHash));
    while(mainWindow->property("questionTextDlgResponse") == -1) {
        QCoreApplication::processEvents();
        QThread::usleep(50);
    }
    if (mainWindow->property("questionTextDlgResponse") == 1) {
//        if (ttl_blocks != nullptr) {
//            *ttl_blocks = mainWindow->property("ttl_blocks").toInt();
//        }
        return WndManager::RETURN_CODE::BTN2;
    } else
        return WndManager::RETURN_CODE::BTN1;
}

// QFileDialog::getSaveFileName call
QString MobileWndManager::getSaveFileName(const QString &caption, const QString &dir, const QString &filter) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::getSaveFileName with caption=" + caption +
        " dir=" + dir + " filter=" + filter);
    Q_UNUSED(caption)
    Q_UNUSED(dir)
    Q_UNUSED(filter)

    QDateTime now;
    QString fileName = downloadPath + "/" + now.currentDateTime().toString("MMMM-d-yyyy-hh-mm-ss");
    return fileName;
}

// QFileDialog::getLoadFileName call
// Mobile migth never need that.
QString MobileWndManager::getOpenFileName(const QString &caption, const QString &dir, const QString &filter) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::getOpenFileName with caption=" + caption +
        " dir=" + dir + " filter=" + filter);
    Q_UNUSED(caption)
    Q_UNUSED(dir)
    Q_UNUSED(filter)

    Q_ASSERT(false); // implement me
    return "";
}


// Ask for confirmation
bool MobileWndManager::sendConfirmationDlg( QString title, QString message, double widthScale, QString passwordHash ) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::sendConfirmationDlg with title=" + title +
        " message=" + message + " passwordHash=<hidden>");
    Q_UNUSED(widthScale)

    if(mainWindow) {
        mainWindow->setProperty("sendConformationDlgResponse", -1);
        QVariant retValue;
        QMetaObject::invokeMethod(mainWindow, "openSendConfirmationDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, title), Q_ARG(QVariant, message), Q_ARG(QVariant, passwordHash));
        while(mainWindow->property("sendConformationDlgResponse") == -1) {
            QCoreApplication::processEvents();
            QThread::usleep(50);
        }
        return mainWindow->property("sendConformationDlgResponse") == 1;
    }
    return false;
}

// Stopping wallet message
void MobileWndManager::showWalletStoppingMessage(int taskTimeout) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::showWalletStoppingMessage with taskTimeout=" + QString::number(taskTimeout));
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "openWalletStoppingMessageDlg", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, taskTimeout));
}

void MobileWndManager::hideWalletStoppingMessage() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::hideWalletStoppingMessage");
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "closeWalletStoppingMessageDlg", Q_RETURN_ARG(QVariant, retValue));
}

//---------------- Pages ------------------------
void MobileWndManager::pageInitFirstTime() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageInitFirstTime");
    //Q_ASSERT(false); // implement me
}
void MobileWndManager::pageInputPassword(QString pageTitle, bool lockMode) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageInputPassword with pageTitle=" + pageTitle +
        " lockMode=" + QString::number(lockMode));
    Q_UNUSED(pageTitle)

    QJsonObject obj;
    obj["lockMode"] = lockMode;
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
    mainWindow->setProperty("currentState", state::STATE::INPUT_PASSWORD);
}
void MobileWndManager::pageInitAccount(QString path, bool restoredFromSeed) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageInitAccount with path=" + path +
        " restoredFromSeed=" + QString::number(restoredFromSeed));
    Q_UNUSED(path) // mobile doesn't need it
    Q_UNUSED(restoredFromSeed)  // mobile doesn't need it
    //Q_ASSERT(false); // implement me
}
void MobileWndManager::pageEnterSeed() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageEnterSeed");
    QJsonObject obj;
    obj["currentStep"] = 3;
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
void MobileWndManager::pageNewSeed(QString pageTitle, QVector<QString> seed, bool hideSubmitButton) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageNewSeed with pageTitle=" + pageTitle +
        " hideSubmitButton=" + QString::number(hideSubmitButton));
    Q_UNUSED(pageTitle)

    QJsonObject obj;
    QString strSeed = "";
    for (int i = 0; i < seed.length() ; i++) {
        strSeed += seed.at(i) + " ";
    }
    obj["currentStep"] = 1;
    obj["seed"] = strSeed;
    obj["hideSubmitButton"] = hideSubmitButton;
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
void MobileWndManager::pageNewSeedTest(int wordIndex) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageNewSeedTest with wordIndex=" + QString::number(wordIndex));
    QJsonObject obj;
    obj["currentStep"] = 2;
    obj["wordIndex"] = wordIndex;
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
void MobileWndManager::pageProgressWnd(QString pageTitle, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageProgressWnd with pageTitle=" + pageTitle +
        " callerId=" + callerId + " header=" + header + " msgProgress=" + msgProgress + " msgPlus=" + msgPlus +
        " cancellable=" + QString::number(cancellable));
    Q_UNUSED(cancellable)

    if (pageTitle.contains("Re-sync")) {
        mainWindow->setProperty("currentState", state::STATE::RESYNC);
    }

    QJsonObject obj;
    obj["callerId"] = callerId;
    obj["header"] = header;
    obj["msgProgress"] = msgProgress;
    obj["msgPlus"] = msgPlus;
    QVariant retValue;
    QMetaObject::invokeMethod(mainWindow, "updateInitParams", Q_RETURN_ARG(QVariant, retValue), Q_ARG(QVariant, QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
void MobileWndManager::pageOutputs() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageOutputs");
}
void MobileWndManager::pageFileTransactionReceive(QString pageTitle,
                                     const QString & fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
                                     int nodeHeight) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageFileTransactionReceive with pageTitle=" + pageTitle +
        " <fileNameOrSlatepack> transInfo.transactionId=" + transInfo.transactionId + " nodeHeight=" + QString::number(nodeHeight));
    Q_UNUSED(pageTitle)

    QJsonObject obj;
    obj["fileNameOrSlatepack"] = fileNameOrSlatepack;
    obj["amount"] = util::nano2one(transInfo.amount);
    obj["transactionId"] = transInfo.transactionId;
    obj["lockHeight"] = transInfo.lock_height > nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-";
    obj["senderAddress"] = transInfo.fromAddress.isEmpty() ? "-" : transInfo.fromAddress;
    obj["senderMessage"] = transInfo.senderMessage;
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void MobileWndManager::pageFileTransactionFinalize(QString pageTitle,
                                           const QString & fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
                                           int nodeHeight) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageFileTransactionFinalize with pageTitle=" + pageTitle +
        " fileNameOrSlatepack=<hidden> transInfo.transactionId=" + transInfo.transactionId +
        " nodeHeight=" + QString::number(nodeHeight));
    Q_UNUSED(pageTitle)
    QJsonObject obj;
    obj["fileNameOrSlatepack"] = fileNameOrSlatepack;
    obj["amount"] = util::nano2one(transInfo.amount);
    obj["transactionId"] = transInfo.transactionId;
    obj["lockHeight"] = transInfo.lock_height > nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-";
    obj["senderAddress"] = transInfo.fromAddress.isEmpty() ? "-" : transInfo.fromAddress;
    obj["senderMessage"] = transInfo.senderMessage;
    obj["receiverMessage"] = transInfo.receiverMessage;
    obj["amount_fee_not_defined"] = transInfo.amount_fee_not_defined;
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void MobileWndManager::pageRecieve() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageRecieve");
    mainWindow->setProperty("currentState", state::STATE::RECEIVE_COINS);
    mainWindow->setProperty("initParams", "");
}
void MobileWndManager::pageListening() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageListening");
//    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageFinalize() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageFinalize");
    mainWindow->setProperty("currentState", state::STATE::FINALIZE);
    mainWindow->setProperty("initParams", "");
}
void MobileWndManager::pageSendStarting() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSendStarting");
    mainWindow->setProperty("currentState", state::STATE::SEND);
    mainWindow->setProperty("initParams", "");
}
void MobileWndManager::pageSendOnline( QString selectedAccount, qint64 amount ) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSendOnline with selectedAccount=" + selectedAccount +
        " amount=" + QString::number(amount));
    QJsonObject obj;
    obj["isSendOnline"] = true;
    obj["selectedAccount"] = selectedAccount;
    obj["amount"] = QString::number(amount);
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
}
void MobileWndManager::pageSendFile( QString selectedAccount, qint64 amount ) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSendFile with selectedAccount=" + selectedAccount + " amount=" + QString::number(amount));
    QJsonObject obj;
    obj["isSendOnline"] = false;
    obj["selectedAccount"] = selectedAccount;
    obj["amount"] = QString::number(amount);
    obj["isSlatepack"] = false;
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void MobileWndManager::pageSendSlatepack( QString selectedAccount, qint64 amount ) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSendSlatepack with selectedAccount=" + selectedAccount +
        " amount=" + QString::number(amount));
    QJsonObject obj;
    obj["isSendOnline"] = false;
    obj["selectedAccount"] = selectedAccount;
    obj["amount"] = QString::number(amount);
    obj["isSlatepack"] = true;
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void MobileWndManager::pageTransactions() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageTransactions");
    mainWindow->setProperty("currentState", state::STATE::TRANSACTIONS);
}

// slatepack - slatepack string value to show.
// backStateId - state ID of the caller. On 'back' will switch to this state Id
void MobileWndManager::pageShowSlatepack(QString slatepack, int backStateId, QString txExtension, bool enableFinalize) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageShowSlatepack with <slatepack> backStateId=" + QString::number(backStateId) +
        " txExtension=" + txExtension + " enableFinalize=" + QString::number(enableFinalize));
    QJsonObject obj;
    obj["slatepack"] = slatepack;
    obj["backStateId"] = backStateId;
    obj["txExtension"] = txExtension;
    obj["enableFinalize"] = enableFinalize;
    mainWindow->setProperty("initParams", QJsonDocument(obj).toJson(QJsonDocument::Compact));
}


void MobileWndManager::pageAccounts() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageAccounts");
}
void MobileWndManager::pageAccountTransfer() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageAccountTransfer");
}
void MobileWndManager::pageNodeInfo() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageNodeInfo");
}
void MobileWndManager::pageContacts() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageContacts");
}
void MobileWndManager::pageEvents() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageEvents");
}
void MobileWndManager::pageWalletConfig() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageWalletConfig");
}
void MobileWndManager::pageNodeConfig() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageNodeConfig");
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageSelectMode() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSelectMode");
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageWalletHome() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageWalletHome");
    mainWindow->setProperty("currentState", state::STATE::WALLET_HOME);
}

void MobileWndManager::pageWalletSettings() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageWalletSettings");
    mainWindow->setProperty("currentState", state::STATE::WALLET_SETTINGS);
}

void MobileWndManager::pageAccountOptions() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageAccountOptions");
    mainWindow->setProperty("currentState", state::STATE::ACCOUNT_OPTIONS);
}

void MobileWndManager::pageSwapList(bool selectIncoming, bool selectOutgoing, bool selectBackup, bool selectEthWallet) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSwapList with selectIncoming=" + QString::number(selectIncoming) + " selectOutgoing=" + QString::number(selectOutgoing) + " selectBackup=" + QString::number(selectBackup) + " selectEthWallet=" + QString::number(selectEthWallet));
    Q_UNUSED(selectIncoming)
    Q_UNUSED(selectOutgoing)
    Q_UNUSED(selectBackup)
    Q_UNUSED(selectEthWallet)
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageSwapNew1() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSwapNew1");
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageSwapNew2() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSwapNew2");
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageSwapNew3() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSwapNew3");
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageSwapEdit(QString swapId, QString stateCmd) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSwapEdit with swapId=" + swapId + " stateCmd=" + stateCmd);
    Q_UNUSED(swapId)
    Q_UNUSED(stateCmd)
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageSwapTradeDetails(QString swapId) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageSwapTradeDetails with swapId=" + swapId);
    Q_UNUSED(swapId)
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageMarketplace(bool selectMyOffers, bool selectFee) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageMarketplace with selectMyOffers=" + QString::number(selectMyOffers) + " selectFee=" + QString::number(selectFee));
    Q_UNUSED(selectMyOffers)
    Q_UNUSED(selectFee)
    Q_ASSERT(false); // implement me
}
void MobileWndManager::pageNewUpdateOffer(QString myMsgId) {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageNewUpdateOffer with myMsgId=" + myMsgId);
    Q_UNUSED(myMsgId)
    Q_ASSERT(false); // implement me
}

void MobileWndManager::showSwapBackupDlg() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::showSwapBackupDlg");
    Q_ASSERT(false); // implement me
}

void MobileWndManager::pageTransactionFee() {
    logger::logInfo(logger::QT_WALLET, "Call MobileWndManager::pageTransactionFee");
    Q_ASSERT(false); // implement me
}



}

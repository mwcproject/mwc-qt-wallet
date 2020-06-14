// Copyright 2019 The MWC Developers
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

#include "u_nodeinfo_w.h"
#include "ui_u_nodeinfo_w.h"
#include "../control_desktop/messagebox.h"
#include "../dialogs_desktop/u_changenode.h"
#include <QScrollBar>
#include "../util_desktop/timeoutlock.h"
#include "../dialogs_desktop/u_mwcnodelogs.h"
#include <QFileDialog>
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/wnd/u_nodeInfo_b.h"
#include "../core/global.h"
#include "../bridge/util_b.h"

namespace wnd {

//static
QString NodeInfo::lastShownErrorMessage;

static QString toBoldAndYellow(QString text) {
    if (text == "Ready")
        return text;

    return "<span style=\" font-weight:900; color:#CCFF33;\">" + text + "</span>";
}


NodeInfo::NodeInfo(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::NodeInfo)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    wallet = new bridge::Wallet(this);
    nodeInfo = new bridge::NodeInfo(this);
    util = new bridge::Util(this);

    // Need simulate post message. Using events for that
    connect(this, &NodeInfo::showNodeConnectionError, this,  &NodeInfo::onShowNodeConnectionError, Qt::QueuedConnection );
    connect(nodeInfo, &bridge::NodeInfo::sgnSetNodeStatus, this, &NodeInfo::onSgnSetNodeStatus, Qt::QueuedConnection );
    connect(nodeInfo, &bridge::NodeInfo::sgnUpdateEmbeddedMwcNodeStatus, this, &NodeInfo::onSgnUpdateEmbeddedMwcNodeStatus, Qt::QueuedConnection );
    connect(nodeInfo, &bridge::NodeInfo::sgnHideProgress, this, &NodeInfo::onSgnHideProgress, Qt::QueuedConnection );

    ui->warningLine->hide();

    ui->progress->initLoader(false);

    connectionType = wallet::MwcNodeConnection::fromJson(nodeInfo->getNodeConnection()).connectionType;

    if (connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL)
        ui->statusInfo->setText( toBoldAndYellow( nodeInfo->getMwcNodeStatus() ) );

    ui->showLogsButton->setEnabled( connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL );

    if ( !config->isOnlineWallet() )
        ui->onlineWalletBtns->hide();

    if ( !config->isOnlineNode() )
        ui->onlineNodeBtns->hide();

    if ( !config->isColdWallet() )
        ui->coldWalletBtns->hide();

    updateNodeReadyButtons(false);

    showWarning("");
}

NodeInfo::~NodeInfo() {
    delete ui;
}

// logs to show, multi like output
void NodeInfo::onSgnUpdateEmbeddedMwcNodeStatus( QString status ) {
    if (connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL)
        ui->statusInfo->setText( toBoldAndYellow(status) );
}

// Empty string to hide warning...
void NodeInfo::showWarning(QString warning) {
    if (currentWarning == warning)
        return;

    currentWarning = warning;

    if (warning.isEmpty()) {
        ui->lineSeparator->show();
        ui->warningLine->hide();
    }
    else {
        ui->lineSeparator->hide();
        ui->warningLine->show();
        ui->warningLine->setText(warning);
    }
}


void NodeInfo::onSgnSetNodeStatus( QString localNodeStatus,
                                 bool online,  QString errMsg, int nodeHeight, int peerHeight,
                                 QString totalDifficulty2show, int connections) {
    QString warning;

    bool nodeIsReady = false;

    if (!online) {
        QString statusStr = "Offline";

        if (connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL && localNodeStatus!="Ready") {
            statusStr = localNodeStatus;
        }

        ui->statusInfo->setText( toBoldAndYellow(statusStr) );
        ui->connectionsInfo->setText("-");
        ui->heightInfo->setText("-");
        ui->difficultyInfo->setText("-");

        // Don't show message for the local because starting local node might take a while. Error likely will be recoverable
        if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL ) {
            if ( statusStr == "Offline" ) {
                 if (lastShownErrorMessage != errMsg) {
                     emit showNodeConnectionError(errMsg);
                     lastShownErrorMessage = errMsg;
                 }
            }
        }
    }
    else {
        if ( nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < peerHeight ) {
            if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL)
                ui->statusInfo->setText(toBoldAndYellow("Syncing") );
        }
        else {
            if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL)
                ui->statusInfo->setText("Online");

            nodeIsReady = true;
        }

        if (connections <= 0) {
            ui->connectionsInfo->setText( toBoldAndYellow("None") ); // Two offline is confusing and doesn't look good. Let's keep zero and highlight it.

            if (!config->isColdWallet()) {
                if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD) {
                    warning = toBoldAndYellow(
                            "Please note. You can't run two MWC Nodes with same public IP.<br>That might be a reason why node unable to find any peers.");
                }
            }
        }
        else {
            ui->connectionsInfo->setText( QString::number(connections) );
        }

        ui->heightInfo->setText( util->longLong2Str(nodeHeight) );
        ui->difficultyInfo->setText( totalDifficulty2show );
    }

    if (peerHeight==0 || connections==0)
        nodeIsReady = false;

    updateNodeReadyButtons(nodeIsReady);

    showWarning(warning);
}

void NodeInfo::onShowNodeConnectionError(QString errorMessage) {
    control::MessageBox::messageText(this, "MWC Node connection error",
        "Unable to retrieve MWC Node status.\n" + errorMessage);
}

void NodeInfo::on_refreshButton_clicked() {
    if (control::MessageBox::questionText(this, "Re-sync account with a node",
            "Account re-sync will validate transactions and outputs for your accounts. Re-sync can take several minutes.\nWould you like to continue",
            "No", "Yes",
            "Cancel resync operation",
            "Continue with resync, I will wait for a while",
            true, false) == core::WndManager::RETURN_CODE::BTN2 ) {
        nodeInfo->requestWalletResync();
    }
}

void NodeInfo::showNodeLogs() {
    util::TimeoutLockObject to("NodeInfo");

    dlg::MwcNodeLogs logsDlg(this);
    logsDlg.exec();
}

void NodeInfo::on_showLogsButton_clicked() { showNodeLogs(); }
void NodeInfo::on_showLogsButton_5_clicked() { showNodeLogs(); }
void NodeInfo::on_showLogsButton_8_clicked() { showNodeLogs(); }

void NodeInfo::on_changeNodeButton_clicked()
{
    util::TimeoutLockObject to("NodeInfo");

    // call dialog the allow to change the
    wallet::MwcNodeConnection nodeConn = wallet::MwcNodeConnection::fromJson(nodeInfo->getNodeConnection());

    dlg::ChangeNode changeNodeDlg(this, nodeConn, config->getNetwork() );

    if ( changeNodeDlg.exec() == QDialog::Accepted ) {
        nodeInfo->updateNodeConnection( changeNodeDlg.getNodeConnectionConfig().toJson() );
    }
}

void NodeInfo::onSgnHideProgress() {
    ui->progress->hide();
}

void NodeInfo::on_saveBlockchianData_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Blockchain Data"),
                                                       config->getPathFor("BlockchainData"),
                                                       tr("MWC Blockchain Data (*.mwcblc)"));

    if (fileName.length()==0)
          return;
    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return;
    }

    if (!fileName.endsWith(".mwcblc"))
        fileName += ".mwcblc";

    ui->progress->show();
    config->updatePathFor("BlockchainData", QFileInfo(fileName).absolutePath() );
    nodeInfo->exportBlockchainData(fileName);
}

void NodeInfo::on_loadBlockchainData_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Blockchain Data"),
                                                    config->getPathFor("BlockchainData"),
                                                    tr("MWC Blockchain Data (*.mwcblc);;All files (*.*)"));

    if (fileName.length()==0)
          return;
    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return;
    }

    ui->progress->show();
    config->updatePathFor("BlockchainData", QFileInfo(fileName).absolutePath() );
    nodeInfo->importBlockchainData(fileName);
}

void NodeInfo::on_publishTransaction_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Publish MWC transaction"),
                                                    config->getPathFor("PublishTransaction"),
                                                    tr("MWC transaction (*.mwctx)"));

    if (fileName.length()==0)
          return;
    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return;
    }

    ui->progress->show();
    config->updatePathFor("PublishTransaction", QFileInfo(fileName).absolutePath() );
    nodeInfo->publishTransaction(fileName);
}

void NodeInfo::updateNodeReadyButtons(bool nodeIsReady) {
    ui->publishTransaction->setEnabled(nodeIsReady);
    ui->saveBlockchianData->setEnabled(nodeIsReady);
    ui->refreshButton->setEnabled(nodeIsReady);
}

}

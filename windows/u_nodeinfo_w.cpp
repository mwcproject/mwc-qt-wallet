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

#include <core/global.h>
#include "u_nodeinfo_w.h"
#include "ui_u_nodeinfo_w.h"
#include "../state/u_nodeinfo.h"
#include "../control/messagebox.h"
#include "../dialogs/u_changenode.h"
#include <QScrollBar>
#include "../state/timeoutlock.h"
#include "../dialogs/u_mwcnodelogs.h"
#include "../core/Config.h"
#include <QFileDialog>

namespace wnd {

//static
QString NodeInfo::lastShownErrorMessage;

static QString toBoldAndYellow(QString text) {
    if (text == "Ready")
        return text;

    return "<span style=\" font-weight:900; color:#CCFF33;\">" + text + "</span>";
}


NodeInfo::NodeInfo(QWidget *parent, state::NodeInfo * _state) :
        core::NavWnd(parent, _state->getContext()),
        ui(new Ui::NodeInfo),
        state(_state)
{
    ui->setupUi(this);

    ui->warningLine->hide();

    ui->progress->initLoader(false);

    // Need simulate post message. Using events for that
    connect(this, &NodeInfo::showNodeConnectionError, this,  &NodeInfo::onShowNodeConnectionError, Qt::QueuedConnection );

    connectionType = state->getNodeConnection().first.connectionType;

    if (connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL)
        ui->statusInfo->setText( toBoldAndYellow( state->getMwcNodeStatus() ) );

    ui->showLogsButton->setEnabled( connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL );

    if ( !config::isOnlineWallet() )
        ui->onlineWalletBtns->hide();

    if ( !config::isOnlineNode() )
        ui->onlineNodeBtns->hide();

    if ( !config::isColdWallet() )
        ui->coldWalletBtns->hide();

    updateNodeReadyButtons(false);

    showWarning("");
}

NodeInfo::~NodeInfo() {
    state->wndIsGone(this);
    delete ui;
}

// logs to show, multi like output
void NodeInfo::updateEmbeddedMwcNodeStatus( const QString & status ) {
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


void NodeInfo::setNodeStatus( const state::NodeStatus & status ) {
    QString warning;

    bool nodeIsReady = false;

    if (!status.online) {
        ui->statusInfo->setText( toBoldAndYellow("Offline") );
        ui->connectionsInfo->setText("-");
        ui->heightInfo->setText("-");
        ui->difficultyInfo->setText("-");

        // Don't show message for the local because starting local node might take a while. Error likely will be recoverable
        if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL ) {
            if (lastShownErrorMessage != status.errMsg) {
                 emit showNodeConnectionError(status.errMsg);
                 lastShownErrorMessage = status.errMsg;
            }
        }
    }
    else {
        if ( status.nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < status.peerHeight ) {
            if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL)
                ui->statusInfo->setText(toBoldAndYellow("Syncing") );
        }
        else {
            if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL)
                ui->statusInfo->setText("Online");

            nodeIsReady = true;
        }

        if (status.connections <= 0) {
            nodeIsReady = false;
            ui->connectionsInfo->setText( toBoldAndYellow("None") ); // Two offline is confusing and doesn't look good. Let's keep zero and highlight it.

            if (!config::isColdWallet()) {
                if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD) {
                    warning = toBoldAndYellow(
                            "Please note. You can't run two mwc-node with same public IP.<br>That might be a reason why node unable to find any peers.");
                }
            }
        }
        else {
            ui->connectionsInfo->setText( util::longLong2Str(status.connections) );
        }

        ui->heightInfo->setText( util::longLong2Str(status.nodeHeight) );
        ui->difficultyInfo->setText( util::longLong2ShortStr(status.totalDifficulty, 9) );
    }

    updateNodeReadyButtons(nodeIsReady);

    showWarning(warning);
}

void NodeInfo::onShowNodeConnectionError(QString errorMessage) {
    control::MessageBox::message(this, "mwc node connection error",
        "Unable to retrieve mwc node status.\n" + errorMessage);
}

void NodeInfo::on_refreshButton_clicked() {
    if (control::MessageBox::question(this, "Re-sync account with a node", "Account re-sync will validate transactions and outputs for your accounts. Re-sync can take several minutes.\nWould you like to continue",
                       "Yes", "No", false,true) == control::MessageBox::RETURN_CODE::BTN1 ) {
        state->requestWalletResync();
    }
}

void NodeInfo::showNodeLogs() {
    state::TimeoutLockObject to( state );

    dlg::MwcNodeLogs logsDlg(this, state->getMwcNode() );
    logsDlg.exec();
}

void NodeInfo::on_showLogsButton_clicked() { showNodeLogs(); }
void NodeInfo::on_showLogsButton_5_clicked() { showNodeLogs(); }
void NodeInfo::on_showLogsButton_8_clicked() { showNodeLogs(); }

void NodeInfo::on_changeNodeButton_clicked()
{
    state::TimeoutLockObject to( state );

    // call dialog the allow to change the
    QPair< wallet::MwcNodeConnection, wallet::WalletConfig > conInfo = state->getNodeConnection();

    dlg::ChangeNode changeNodeDlg(this, conInfo.first, conInfo.second.getNetwork() );

    if ( changeNodeDlg.exec() == QDialog::Accepted ) {
        state->updateNodeConnection( changeNodeDlg.getNodeConnectionConfig(), conInfo.second );
    }
}

void NodeInfo::hideProgress() {
    ui->progress->hide();
}

void NodeInfo::on_saveBlockchianData_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Blockchain Data"),
                                                       state->getBlockchainDataPath(),
                                                       tr("MWC Blockchain Data (*.mwcblc)"));

    if (fileName.length()==0)
          return;

    ui->progress->show();
    state->updateBlockchainDataPath( QFileInfo(fileName).absolutePath() );
    state->saveBlockchainData(fileName);
}

void NodeInfo::on_loadBlockchainData_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Blockchain Data"),
                                                       state->getBlockchainDataPath(),
                                                       tr("MWC Blockchain Data (*.mwcblc)"));

    if (fileName.length()==0)
          return;

    ui->progress->show();
    state->updateBlockchainDataPath( QFileInfo(fileName).absolutePath() );
    state->loadBlockchainData(fileName);
}

void NodeInfo::on_publishTransaction_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Publish MWC transaction"),
                                                       state->getPublishTransactionPath(),
                                                       tr("MWC transaction (*.mwctx)"));

    if (fileName.length()==0)
          return;

    ui->progress->show();
    state->updatePublishTransactionPath( QFileInfo(fileName).absolutePath() );
    state->publishTransaction(fileName);
}

void NodeInfo::updateNodeReadyButtons(bool nodeIsReady) {
    ui->publishTransaction->setEnabled(nodeIsReady);
    ui->saveBlockchianData->setEnabled(nodeIsReady);
    ui->refreshButton->setEnabled(nodeIsReady);
}

}

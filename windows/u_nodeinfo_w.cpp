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

namespace wnd {

//static
QString NodeInfo::lastShownErrorMessage;


NodeInfo::NodeInfo(QWidget *parent, state::NodeInfo * _state) :
        core::NavWnd(parent, _state->getContext()),
        ui(new Ui::NodeInfo),
        state(_state)
{
    ui->setupUi(this);

    ui->warningLine->hide();

    // Need simulate post message. Using events for that
    connect(this, &NodeInfo::showNodeConnectionError, this,  &NodeInfo::onShowNodeConnectionError, Qt::QueuedConnection );

    connectionType = state->getNodeConnection().first.connectionType;

    ui->embeddedNodeStatus->setText( state->getMwcNodeStatus() );

    if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL) {
        ui->embeddedNodeStatus->hide();
    }

    showWarning("");
}

NodeInfo::~NodeInfo() {
    state->wndIsGone(this);
    delete ui;
}


// logs to show, multi like output
void NodeInfo::updateEmbeddedMwcNodeStatus( const QString & status ) {
    ui->embeddedNodeStatus->setText(status);
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


static QString toBoldAndYellow(QString text) {
    return "<span style=\" font-weight:900; color:#CCFF33;\">" + text + "</span>";
}

void NodeInfo::setNodeStatus( const state::NodeStatus & status ) {
    QString warning;

    if (!status.online) {
        ui->statusInfo->setText( toBoldAndYellow("Offline") );
        ui->connectionsInfo->setText("-");
        ui->heightInfo->setText("-");
        ui->difficultyInfo->setText("-");

         if (lastShownErrorMessage != status.errMsg) {
             emit showNodeConnectionError(status.errMsg);
             lastShownErrorMessage = status.errMsg;
        }
    }
    else {
        if ( status.nodeHeight + mwc::NODE_HEIGHT_DIFF_LIMIT < status.peerHeight )
            ui->statusInfo->setText(toBoldAndYellow("Syncing") );
        else
            ui->statusInfo->setText("Online");

        if (status.connections <= 0) {
            ui->connectionsInfo->setText( toBoldAndYellow("None") ); // Two offline is confusing and doesn't look good. Let's keep zero and highlight it.

            if (connectionType != wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD) {
                warning = toBoldAndYellow("Please note. You can't run two mwc-node with same public IP.<br>That might be a reason why node unable to find any peers.");
            }
        }
        else {
            ui->connectionsInfo->setText( util::longLong2Str(status.connections) );
        }

        ui->heightInfo->setText( util::longLong2Str(status.nodeHeight) );
        ui->difficultyInfo->setText( util::longLong2ShortStr(status.totalDifficulty, 9) );
    }

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

void NodeInfo::on_chnageNodeButton_clicked() {
    state::TimeoutLockObject to( state );

    // call dialog the allow to change the
    QPair< wallet::MwcNodeConnection, wallet::WalletConfig > conInfo = state->getNodeConnection();

    dlg::ChangeNode changeNodeDlg(this, conInfo.first, conInfo.second.getNetwork() );

    if ( changeNodeDlg.exec() == QDialog::Accepted ) {
        state->updateNodeConnection( changeNodeDlg.getNodeConnectionConfig(), conInfo.second );
    }
}

}



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
#include "../state/u_nodeinfo.h"
#include "../control/messagebox.h"
#include "../dialogs/u_changenode.h"

namespace wnd {

NodeInfo::NodeInfo(QWidget *parent, state::NodeInfo * _state) :
        core::NavWnd(parent, _state->getContext()),
        ui(new Ui::NodeInfo),
        state(_state)
{
    ui->setupUi(this);

    // progress is active because of node info request
    ui->progress->initLoader(true);

    state->requestNodeInfo();
}

NodeInfo::~NodeInfo() {
    state->wndIsGone(this);
    delete ui;
}

void NodeInfo::setNodeStatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections ) {
    ui->progress->hide();

    if (!online) {
        ui->statusInfo->setText("Offline");
        ui->connectionsInfo->setText("-");
        ui->heightInfo->setText("-");
        ui->difficultyInfo->setText("-");

        control::MessageBox::message( this, "mwc node connection error", "Unable to connect to mwc node.\n" + errMsg );
    }
    else {
        ui->statusInfo->setText("Online");
        ui->connectionsInfo->setText( util::longLong2Str(connections) );
        ui->heightInfo->setText( util::longLong2Str(height) );
        ui->difficultyInfo->setText( util::longLong2Str(totalDifficulty) );
    }
}


void NodeInfo::on_refreshButton_clicked() {
    ui->progress->show();
    state->requestNodeInfo();
}

void NodeInfo::on_chnageNodeButton_clicked() {
    // call dialog the allow to change the
    dlg::ChangeNode changeNodeDlg(this, state->getWalletConfig() );

    if ( changeNodeDlg.exec() == QDialog::Accepted ) {
        state->updateWalletConfig( changeNodeDlg.getConfig() );
    }
}

}



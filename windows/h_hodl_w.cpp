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

#include "h_hodl_w.h"
#include "ui_h_hodl.h"
#include "../state/h_hodl.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include "../util/execute.h"
#include "../core/HodlStatus.h"

namespace wnd {

Hodl::Hodl(QWidget *parent, state::Hodl * _state) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::Hodl),
    state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);
    ui->claimMwcButton->setEnabled(false);

    updateHodlState();
}

Hodl::~Hodl()
{
    state->deleteHodlWnd(this);
    delete ui;
}

void Hodl::on_learnMoreButton_clicked()
{
    util::openUrlInBrowser("https://www.mwc.mw/hodl");
}

void Hodl::on_signInButton_clicked()
{
    state->registerAccountForHODL();
    ui->progress->show();
}

void Hodl::on_claimMwcButton_clicked()
{
    state->moveToClaimPage();
}

void Hodl::reportMessage(const QString & title, const QString & message) {
    state::TimeoutLockObject to( state );
    ui->progress->hide();

    control::MessageBox::messageText(this, title, message);
}

// Hodl object changed it's state, need to refresh
void Hodl::updateHodlState() {
    ui->hodlStatus->setText( state->getContext()->hodlStatus->getHodlStatus() );
    ui->accountStatus->setText( state->getContext()->hodlStatus->getAccountStatus() );
}



}



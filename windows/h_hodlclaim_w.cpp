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

#include "h_hodlclaim_w.h"
#include "ui_h_hodlclaim_w.h"
#include "../state/h_hodl.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include "../core/HodlStatus.h"

namespace wnd {

HodlClaim::HodlClaim(QWidget *parent, state::Hodl * _state) :
        core::NavWnd( parent, _state->getContext() ),
        ui(new Ui::HodlClaim),
        state(_state) {
    ui->setupUi(this);

    ui->progress->initLoader(false);

    initTableHeaders();
    updateClaimsRequests();
}

HodlClaim::~HodlClaim() {
    state->deleteHodlClaimWnd(this);
    saveTableHeaders();
    delete ui;
}


void HodlClaim::reportMessage(const QString & title, const QString & message) {
    state::TimeoutLockObject to( state );
    ui->progress->hide();

    control::MessageBox::messageText(this, title, message);
}

// Hodl object changed it's state, need to refresh
void HodlClaim::updateHodlState() {
    ui->accountStatus->setText( state->getContext()->hodlStatus->getAccountStatus() );
}

void HodlClaim::on_claimMwcButton_clicked()
{
    state->claimMWC();
}

void HodlClaim::updateClaimsRequests() {
    QVector<core::HodlClaimStatus> status = state->getContext()->hodlStatus->getClaimsRequestStatus();

    ui->claimsTable->clearData();

    int idx = 0;
    for (const auto & st : status) {
        ui->claimsTable->appendRow(
                QVector<QString>{QString::number(++idx),
                                 util::nano2one(st.HodlAmount),
                                 util::nano2one(st.claimedMwc),
                                 st.date});
    }

}

void HodlClaim::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 5 ) {
        widths = QVector<int>{30,150,150,300,300};
    }
    Q_ASSERT( widths.size() == 5 );
    ui->claimsTable->setColumnWidths( widths );
}

void HodlClaim::saveTableHeaders() {
    state->updateColumnsWidhts( ui->claimsTable->getColumnWidths() );
}


}


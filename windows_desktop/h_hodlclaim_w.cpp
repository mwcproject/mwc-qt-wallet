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

HodlClaim::HodlClaim(QWidget *parent, state::Hodl * _state, const QString & _coldWalletHash) :
        core::NavWnd( parent, _state->getContext() ),
        ui(new Ui::HodlClaim),
        state(_state),
        coldWalletHash(_coldWalletHash)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    initTableHeaders();
    updateHodlState();

    if (!coldWalletHash.isEmpty())
        state->requestHodlInfoRefresh(coldWalletHash);
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
    QPair< QString, int64_t> status = state->getContext()->hodlStatus->getWalletHodlStatus(coldWalletHash);

    ui->accountStatus->setText( status.first );

    QString waitingText = "";
    if (status.second>0) {
        waitingText = "Your " + util::nano2one(status.second) + " MWC will be available after finalization. "
                                 "The finalization process may take a while because finalization is done from an offline wallet and done in batches. "
                                 "For details on the finalization schedule go to http://www.mwc.mw/hodl";
    }
    ui->finalizeWaitingText->setText(waitingText);

    QVector<core::HodlClaimStatus> claimStatus = state->getContext()->hodlStatus->getClaimsRequestStatus(coldWalletHash);

    ui->claimsTable->clearData();

    bool hasClaims = false;

    int idx = 0;
    for (const auto & st : claimStatus) {
        if (st.status<3)
            hasClaims = true;
        ui->claimsTable->appendRow(
                QVector<QString>{QString::number(++idx),
                                 util::nano2one(st.amount),
                                 QString::number(st.claimId),
                                 st.getStatusAsString()});
    }

    ui->claimMwcButton->setEnabled(hasClaims);
}

void HodlClaim::on_claimMwcButton_clicked()
{
    ui->progress->show();
    state->claimMWC(coldWalletHash);
}

void HodlClaim::on_refreshButton_clicked()
{
    state->requestHodlInfoRefresh(coldWalletHash);
}

void HodlClaim::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 4 ) {
        widths = QVector<int>{30,150,150,300};
    }
    Q_ASSERT( widths.size() == 4 );
    ui->claimsTable->setColumnWidths( widths );
}

void HodlClaim::saveTableHeaders() {
    state->updateColumnsWidhts( ui->claimsTable->getColumnWidths() );
}


}


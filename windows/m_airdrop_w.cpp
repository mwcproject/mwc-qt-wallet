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

#include "m_airdrop_w.h"
#include "ui_m_airdrop.h"
#include "state/m_airdrop.h"
#include "../control/messagebox.h"
#include "../util/stringutils.h"
#include "../state/timeoutlock.h"

namespace wnd {

Airdrop::Airdrop(QWidget *parent, state::Airdrop * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::Airdrop),
    state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    if ( updateAirDropStatus( state->getAirDropStatus() ) ) {
        initTableHeaders();
        updateClaimStatus();
        ui->btcAddressEdit->setFocus();
        ui->claimAirdropBtn->setEnabled(false);
        ui->refreshClaimsButton->setEnabled( state->hasAirdropRequests());
    }
}

Airdrop::~Airdrop()
{
    state->deleteAirdropWnd(this);
    saveTableHeaders();
    delete ui;
}

void Airdrop::on_nextButton_clicked()
{

}

void Airdrop::showProgress(const QString & message) {
    ui->progressMessage->setText(message);
    ui->progressFrame->show();
    ui->claimFrame->hide();
}

void Airdrop::hideProgress() {
    ui->progressFrame->hide();
    ui->claimFrame->show();
}


void Airdrop::on_refreshClaimsButton_clicked()
{
    updateClaimStatus();
}

void Airdrop::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 4 ) {
        widths = QVector<int>{30,300,100,100};
    }
    Q_ASSERT( widths.size() == 4 );
    ui->claimsTable->setColumnWidths( widths );
}

void Airdrop::saveTableHeaders() {
    state->updateColumnsWidhts( ui->claimsTable->getColumnWidths() );
}

void Airdrop::updateClaimStatus() {
    ui->claimsTable->clearData();

    state->refreshAirdropStatusInfo();
}


void Airdrop::on_btcAddressEdit_textChanged(const QString & text)
{
    QString addr = text.trimmed();
    ui->claimAirdropBtn->setEnabled(addr.length()>=26);
}

void Airdrop::on_claimAirdropBtn_clicked()
{
    state::TimeoutLockObject to( state );

    QString address = ui->btcAddressEdit->text().trimmed();

    if (address.length()==0) {
        control::MessageBox::message(this, "Error", "Please input your BTC address that you registered at airdrop");
        return;
    }

    QString password = ui->passwordAirdrop->text();
    if (password.isEmpty()) {
        control::MessageBox::message(this, "Error", "Please specify your password from your Airdrop account at www.mwc.mw");
        return;
    }

    showProgress("Claiming BTC address");

    state->startClaimingProcess( address, password );
}

void Airdrop::reportMessage( QString title, QString message ) {
    state::TimeoutLockObject to( state );

    hideProgress();

    control::MessageBox::message(this, title, message);
}

// true if status is active
bool Airdrop::updateAirDropStatus( const state::AirDropStatus & status ) {
    if (status.status) {
        ui->progressFrame->hide();
        ui->claimFrame->show();
        return true;
    }
    else {
        ui->progressFrame->show();
        ui->claimFrame->hide();

        if (status.waiting)
            ui->progress->show();
        else
            ui->progress->hide();

        ui->progressMessage->setText(status.message);
        return false;
    }
}

bool Airdrop::updateClaimStatus( int idx, const state::AirdropRequests & request,
                        QString status, QString message, int64_t amount, int errCode) {
    Q_UNUSED(message);
    Q_UNUSED(errCode);

    int rows = ui->claimsTable->rowCount();
    if (idx!=rows)
        return false; // go out of sync. Likely double refresh click. We want to keep only once sequence going

    ui->claimsTable->appendRow( QVector<QString>{ QString::number(idx+1), request.btcAddress, amount>0 ? util::nano2one(amount) : "", status } );
    return true;
}



}


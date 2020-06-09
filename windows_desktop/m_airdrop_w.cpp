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
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/m_airdrop_b.h"


namespace wnd {

Airdrop::Airdrop(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Airdrop)
{
    ui->setupUi(this);

    airdrop = new bridge::Airdrop(this);
    config = new bridge::Config(this);

    QObject::connect( airdrop, &bridge::Airdrop::sgnUpdateAirDropStatus,
                      this, &Airdrop::onSgnUpdateAirDropStatus, Qt::QueuedConnection);
    QObject::connect( airdrop, &bridge::Airdrop::sgnUpdateClaimStatus,
                      this, &Airdrop::onSgnUpdateClaimStatus, Qt::QueuedConnection);
    QObject::connect( airdrop, &bridge::Airdrop::sgnReportMessage,
                      this, &Airdrop::onSgnReportMessage, Qt::QueuedConnection);

    ui->progress->initLoader(true);
    ui->progressFrame->hide();
    ui->claimFrame->hide();
}

Airdrop::~Airdrop()
{
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
    QVector<int> widths = config->getColumnsWidhts("AirdropTblWidth");
    if ( widths.size() != 4 ) {
        widths = QVector<int>{30,300,100,100};
    }
    Q_ASSERT( widths.size() == 4 );
    ui->claimsTable->setColumnWidths( widths );
}

void Airdrop::saveTableHeaders() {
    config->updateColumnsWidhts( "AirdropTblWidth", ui->claimsTable->getColumnWidths() );
}

void Airdrop::updateClaimStatus() {
    ui->claimsTable->clearData();

    airdrop->refreshAirdropStatusInfo();
}


void Airdrop::on_btcAddressEdit_textChanged(const QString & text)
{
    QString addr = text.trimmed();
    ui->claimAirdropBtn->setEnabled(addr.length()>=26);
}

void Airdrop::on_claimAirdropBtn_clicked()
{
    util::TimeoutLockObject to("Airdrop");

    QString address = ui->btcAddressEdit->text().trimmed().trimmed();

    if (address.length()==0) {
        control::MessageBox::messageText(this, "Error", "Please input your BTC address that you registered during the airdrop");
        return;
    }

    QString password = ui->passwordAirdrop->text();
    if (password.isEmpty()) {
        control::MessageBox::messageText(this, "Error", "Please specify your password from your Airdrop account at www.mwc.mw");
        return;
    }

    showProgress("Claiming BTC address");

    airdrop->startClaimingProcess( address, password );
}

void Airdrop::onSgnReportMessage( QString title, QString message ) {
    util::TimeoutLockObject to("Airdrop");
    hideProgress();
    control::MessageBox::messageText(this, title, message);
}

// true if status is active
void Airdrop::onSgnUpdateAirDropStatus( bool waiting, bool status, QString message ) {
    if (status) {
        ui->progressFrame->hide();
        ui->claimFrame->show();
    }
    else {
        ui->progressFrame->show();
        ui->claimFrame->hide();

        if (waiting)
            ui->progress->show();
        else
            ui->progress->hide();

        ui->progressMessage->setText(message);
    }
}

void Airdrop::onSgnUpdateClaimStatus( int idx, QString requestBtcAddress,
                                      QString status, QString message, QString mwc, int errCode) {
    Q_UNUSED(message)
    Q_UNUSED(errCode)

    int rows = ui->claimsTable->rowCount();
    if (idx!=rows)
        return; // go out of sync. Likely double refresh click. We want to keep only once sequence going

    ui->claimsTable->appendRow( QVector<QString>{ QString::number(idx+1), requestBtcAddress, mwc, status } );
}



}


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
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/hodlstatus_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/h_hodl_b.h"

namespace wnd {

HodlClaim::HodlClaim(QWidget *parent,const QString & _coldWalletHash) :
        core::NavWnd( parent),
        ui(new Ui::HodlClaim),
        coldWalletHash(_coldWalletHash)
{
    ui->setupUi(this);

    hodl = new bridge::Hodl(this);
    hodlStatus = new bridge::HodlStatus(this);
    config = new bridge::Config(this);

    QObject::connect( hodl, &bridge::Hodl::sgnUpdateHodlState,
                      this, &HodlClaim::onSgnUpdateHodlState, Qt::QueuedConnection);
    QObject::connect( hodl, &bridge::Hodl::sgnReportMessage,
                      this, &HodlClaim::onSgnReportMessage, Qt::QueuedConnection);


    ui->progress->initLoader(false);

    initTableHeaders();
    onSgnUpdateHodlState();

    if (!coldWalletHash.isEmpty())
        hodl->requestHodlInfoRefresh(coldWalletHash);
}

HodlClaim::~HodlClaim() {
    saveTableHeaders();
    delete ui;
}


void HodlClaim::onSgnReportMessage(QString title, QString message) {
    util::TimeoutLockObject to( "HodlClaim" );
    ui->progress->hide();

    control::MessageBox::messageText(this, title, message);
}

// Hodl object changed it's state, need to refresh
void HodlClaim::onSgnUpdateHodlState() {
    QVector<QString> status = hodlStatus->getWalletHodlStatus(coldWalletHash);

    ui->accountStatus->setText( status[0] );

    QString waitingText = "";
    if (status[1] != "0") {
        waitingText = "Your " + status[1] + " MWC will be available after finalization. "
                                 "The finalization process may take a while because finalization is done from an offline wallet and done in batches. "
                                 "For details on the finalization schedule go to http://www.mwc.mw/hodl";
    }
    ui->finalizeWaitingText->setText(waitingText);

    // Lines for Requested status
    // Return groups of 4 [ <true/false is has claim>, <mwc>, <claimId>, <status string> ]
    QVector<QString> claimStatus = hodlStatus->getClaimsRequestStatus(coldWalletHash);

    ui->claimsTable->clearData();

    bool hasClaims = false;

    int idx = 0;
    for ( int i=3; i<claimStatus.size(); i+=4 ) {
        if (claimStatus[i-3]=="true")
            hasClaims = true;
        ui->claimsTable->appendRow(
                QVector<QString>{QString::number(++idx),
                                 claimStatus[i-2],
                                 claimStatus[i-1],
                                 claimStatus[i]});
    }

    ui->claimMwcButton->setEnabled(hasClaims);
}

void HodlClaim::on_claimMwcButton_clicked()
{
    ui->progress->show();
    hodl->claimMWC(coldWalletHash);
}

void HodlClaim::on_refreshButton_clicked() {
    hodl->requestHodlInfoRefresh(coldWalletHash);
}

void HodlClaim::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = config->getColumnsWidhts("HodlTblWidth");
    if ( widths.size() != 4 ) {
        widths = QVector<int>{30,150,150,300};
    }
    Q_ASSERT( widths.size() == 4 );
    ui->claimsTable->setColumnWidths( widths );
}

void HodlClaim::saveTableHeaders() {
    config->updateColumnsWidhts("HodlTblWidth", ui->claimsTable->getColumnWidths());
}


}


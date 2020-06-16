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
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../dialogs_desktop/h_hodlclaimwallet.h"
#include "../bridge/util_b.h"
#include "../bridge/hodlstatus_b.h"
#include "../bridge/wnd/h_hodl_b.h"
#include "../core/HodlStatus.h"

namespace wnd {

Hodl::Hodl(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Hodl)
{
    ui->setupUi(this);

    util = new bridge::Util(this);
    hodl = new bridge::Hodl(this);
    hodlStatus = new bridge::HodlStatus(this);

    QObject::connect( hodl, &bridge::Hodl::sgnUpdateHodlState,
                      this, &Hodl::onSgnUpdateHodlState, Qt::QueuedConnection);
    QObject::connect( hodl, &bridge::Hodl::sgnReportMessage,
                      this, &Hodl::onSgnReportMessage, Qt::QueuedConnection);
    QObject::connect( hodl, &bridge::Hodl::sgnHideWaitingStatus,
                      this, &Hodl::onSgnHideWaitingStatus, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    onSgnUpdateHodlState();
}

Hodl::~Hodl()
{
    delete ui;
}

void Hodl::on_learnMoreButton_clicked()
{
    util->openUrlInBrowser("https://www.mwc.mw/hodl");
}

void Hodl::on_signInButton_clicked()
{
    ui->progress->show();
    hodl->registerAccountForHODL();
}

void Hodl::on_claimMwcButton_clicked()
{
    dlg::HodlClaimWallet claimWalletHashDlg(this);

    QString coldWalletHash;
    if (claimWalletHashDlg.exec() == QDialog::Accepted) {
        coldWalletHash = claimWalletHashDlg.getColdWalletPublicKeyHash();
        hodl->moveToClaimPage(coldWalletHash);
    }
}

void Hodl::onSgnReportMessage(QString title, QString message) {
    util::TimeoutLockObject to( "HODL" );
    ui->progress->hide();
    control::MessageBox::messageText(this, title, message);
}

// Hodl object changed it's state, need to refresh
void Hodl::onSgnUpdateHodlState() {
    QVector<QString> status = hodlStatus->getWalletHodlStatus("");
    Q_ASSERT(status.size()==2);

    ui->signInButton->setEnabled(!hodlStatus->isInHodl(""));
    ui->hodlStatus->setText( hodlStatus->getHodlStatus() );
    ui->accountStatus->setText( status[0] );

    QString waitingText = "";
    if (status[1] != "0") {
        waitingText = "Your " + status[1] + " MWC will be available after finalization. "
                  "The finalization process may take a while because finalization is done from an offline wallet and done in batches. "
                  "For details on the finalization schedule go to http://www.mwc.mw/hodl";
    }
    ui->finalizeWaitingText->setText(waitingText);
}

void Hodl::onSgnHideWaitingStatus() {
    ui->progress->hide();
}

void Hodl::mouseDoubleClickEvent( QMouseEvent * e ) {
    Q_UNUSED(e)

    QString rootPubKey = hodlStatus->getRootPubKey();
    QString rootPubKeyHash =  hodlStatus->getRootPubKeyHash();
    QString isInHodlStr = hodlStatus->isInHodl("") ? "Yes" : "No";

    QVector<QString> jsons = hodlStatus->getHodlOutputs("");
    QVector<core::HodlOutputInfo> hodlOutputs;
    for (auto & j : jsons)
        hodlOutputs.push_back( core::HodlOutputInfo::fromJson(j) );

    double totalHodlAmount = 0.0;
    for (const auto & ho : hodlOutputs)
        totalHodlAmount += ho.value;

    QVector<QString> requestErrors = hodlStatus->getRequestErrors();
    QString requestErrorsStr;
    for ( int r=1; r<requestErrors.size(); r+=2 ) {
        if (requestErrorsStr.length()>0)
            requestErrorsStr+="\n";
        requestErrorsStr += requestErrors[r-1] + " : " + requestErrors[r];
    }

    QString reportStr = "rootpubkey: " + rootPubKey + "\n" +
            "Hash: " + rootPubKeyHash + "\n" +
            "In Hodl status: " + isInHodlStr + "\n" +
            "Hodl amount: " + QString::number(totalHodlAmount) + " at " + QString::number(hodlOutputs.size()) + " outputs\n\n" +
            "API warnings: " + (requestErrorsStr.isEmpty() ? "None" : requestErrorsStr);

    control::MessageBox::messageText(this, "HODL internal status",
            "This is internal state of the wallet HODL. This information intended for MWC developer usage. If you accidentally call this dialog, please close it.\n\n" + reportStr,
            1.3);
}

}



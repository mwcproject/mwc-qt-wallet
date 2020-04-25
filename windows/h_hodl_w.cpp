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
#include "../dialogs/h_hodlclaimwallet.h"

namespace wnd {

Hodl::Hodl(QWidget *parent, state::Hodl * _state) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::Hodl),
    state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    updateHodlState();
}

Hodl::~Hodl()
{
    state->deleteHodlNormWnd(this);
    delete ui;
}

void Hodl::on_learnMoreButton_clicked()
{
    util::openUrlInBrowser("https://www.mwc.mw/hodl");
}

void Hodl::on_signInButton_clicked()
{
    ui->progress->show();
    state->registerAccountForHODL();
}

void Hodl::on_claimMwcButton_clicked()
{
    dlg::HodlClaimWallet claimWalletHashDlg(this);

    QString coldWalletHash;
    if (claimWalletHashDlg.exec() == QDialog::Accepted) {
        coldWalletHash = claimWalletHashDlg.getColdWalletPublicKeyHash();
    }

    state->moveToClaimPage(coldWalletHash);
}

void Hodl::reportMessage(const QString & title, const QString & message) {
    state::TimeoutLockObject to( state );
    ui->progress->hide();

    control::MessageBox::messageText(this, title, message);
}

// Hodl object changed it's state, need to refresh
void Hodl::updateHodlState() {
    core::HodlStatus * hodlStatus = state->getContext()->hodlStatus;
    Q_ASSERT(hodlStatus);

    ui->signInButton->setEnabled(!hodlStatus->isInHodl(""));
    ui->hodlStatus->setText( hodlStatus->getHodlStatus() );
    ui->accountStatus->setText( hodlStatus->getWalletHodlStatus("") );

    // We don't want update Claim button status because it is possibel to claim for
    // another wallet.

    //QVector<core::HodlClaimStatus> claims = hodlStatus->getClaimsRequestStatus();
    //ui->claimMwcButton->setEnabled(!claims.isEmpty());
    //ui->claimMwcButton->setVisible(!claims.isEmpty());
}

void Hodl::hideWaitingStatus() {
    ui->progress->hide();
}

void Hodl::mouseDoubleClickEvent( QMouseEvent * e ) {
    Q_UNUSED(e)

    core::HodlStatus * hodlStatus = state->getContext()->hodlStatus;
    Q_ASSERT(hodlStatus);

    QString rootPubKey = hodlStatus->getRootPubKey();
    QString rootPubKeyHash =  hodlStatus->getRootPubKeyHash();
    QString isInHodlStr = hodlStatus->isInHodl("") ? "Yes" : "No";

    QVector<core::HodlOutputInfo> hodlOutputs = hodlStatus->getHodlOutputs("");
    double totalHodlAmount = 0.0;
    for (const auto & ho : hodlOutputs)
        totalHodlAmount += ho.value;

    QMap<QString, QString> requestErrors = hodlStatus->getRequestErrors();
    QString requestErrorsStr;
    for ( auto re = requestErrors.begin(); re != requestErrors.end(); re++ ) {
        if (requestErrorsStr.length()>0)
            requestErrorsStr+="\n";
        requestErrorsStr += re.key() + " : " + re.value();
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



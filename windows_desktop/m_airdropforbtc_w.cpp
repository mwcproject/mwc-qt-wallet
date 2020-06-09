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

#include "m_airdropforbtc_w.h"
#include "ui_m_airdropforbtc.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wnd/m_airdrop_b.h"

namespace wnd {

AirdropForBTC::AirdropForBTC(QWidget *parent, QString _btcAddress, QString _challenge, QString _identifier) :
    core::PanelBaseWnd(parent),
    ui(new Ui::AirdropForBTC),
    btcAddress(_btcAddress.trimmed()),
    challenge(_challenge.trimmed()),
    identifier(_identifier)
{
    ui->setupUi(this);

    airdrop = new bridge::Airdrop(this);

    QObject::connect( airdrop, &bridge::Airdrop::sgnReportMessage,
                      this, &AirdropForBTC::onSgnReportMessage, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->btcEdit->setText(btcAddress);
    ui->challengeEdit->setText(challenge);

    ui->signatureEdit->setFocus();
}

AirdropForBTC::~AirdropForBTC()
{
    delete ui;
}

void AirdropForBTC::on_claimButton_clicked()
{
    util::TimeoutLockObject to( "AirdropForBTC" );

    QString signature = ui->signatureEdit->toPlainText().trimmed();

    if ( signature.length() == 0 ) {
        control::MessageBox::messageText(this, "MWC claim", "Please input the signature for the challenge");
        return;
    }

    airdrop->requestClaimMWC( btcAddress, challenge, signature, identifier );

    ui->progress->show();
}

void AirdropForBTC::on_backButton_clicked()
{
    airdrop->backToMainAirDropPage();
}

void AirdropForBTC::onSgnReportMessage(QString title, QString message) {
    util::TimeoutLockObject to( "AirdropForBTC" );

    ui->progress->hide();
    control::MessageBox::messageText(this, title, message);
}


}

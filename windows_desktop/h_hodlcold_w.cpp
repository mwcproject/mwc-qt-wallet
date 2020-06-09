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

#include "h_hodlcold_w.h"
#include "ui_h_hodlcold_w.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wnd/h_hodl_b.h"

namespace wnd {

HodlCold::HodlCold(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::HodlCold)
{
    ui->setupUi(this);

    hodl = new bridge::Hodl(this);

    QObject::connect( hodl, &bridge::Hodl::sgnSetRootPubKeyWithSignature,
                      this, &HodlCold::onSgnSetRootPubKeyWithSignature, Qt::QueuedConnection);
    QObject::connect( hodl, &bridge::Hodl::sgnReportMessage,
                      this, &HodlCold::onSgnReportMessage, Qt::QueuedConnection);
    QObject::connect( hodl, &bridge::Hodl::sgnHideWaitingStatus,
                      this, &HodlCold::onSgnHideWaitingStatus, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->sign->setEnabled(false);

    // waiting for the public key
    ui->progress->show();
}

HodlCold::~HodlCold()
{
    delete ui;
}

void HodlCold::onSgnSetRootPubKeyWithSignature(QString key, QString hash, QString message, QString signature) {
    ui->progress->hide();
    ui->publicKey->setText(key);
    ui->publicKeyHash->setText(hash);
    if (ui->message->text()==message) {
        ui->signature->setPlainText(signature);
    }
}

void HodlCold::onSgnReportMessage( QString title, QString message) {
    util::TimeoutLockObject to( "HodlCold" );
    ui->progress->hide();

    control::MessageBox::messageText(this, title, message);
}

void HodlCold::on_message_textChanged(const QString &str)
{
    ui->sign->setEnabled(str.length()>0);
    ui->signature->setPlainText("");
}

void HodlCold::on_sign_clicked()
{
    ui->progress->show();
    hodl->requestSignMessage(ui->message->text());
}

void HodlCold::onSgnHideWaitingStatus() {
    ui->progress->hide();
}


}


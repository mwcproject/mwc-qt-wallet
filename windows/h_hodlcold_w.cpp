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
#include "../state/h_hodl.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include "../util/crypto.h"

namespace wnd {

HodlCold::HodlCold(QWidget *parent, state::Hodl * _state) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::HodlCold),
    state(_state)
{
    ui->setupUi(this);
    ui->progress->initLoader(false);

    ui->sign->setEnabled(false);

    // waiting for the public key
    ui->progress->show();
}

HodlCold::~HodlCold()
{
    state->deleteHodlColdWnd(this);
    delete ui;
}

void HodlCold::setRootPubKeyWithSignature(const QString & key, const QString & message, const QString & signature) {
    ui->progress->hide();
    ui->publicKey->setText(key);
    QByteArray keyHex = key.toUtf8();
    ui->publicKeyHash->setText( crypto::hex2str( crypto::HSA256( keyHex ) ) );
    if (ui->message->text()==message) {
        ui->signature->setPlainText(signature);
    }
}

void HodlCold::reportMessage(const QString & title, const QString & message) {
    state::TimeoutLockObject to( state );
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
    state->requestSignMessage(ui->message->text());
}

void HodlCold::hideWaitingStatus() {
    ui->progress->hide();
}


}


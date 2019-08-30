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

#include "e_listening_w.h"
#include "ui_e_listening.h"
#include "../state/e_listening.h"
#include "../control/messagebox.h"
#include "../control/inputdialog.h"
#include <QDebug>
#include "../state/timeoutlock.h"

namespace wnd {

Listening::Listening(QWidget *parent, state::Listening * _state, bool mwcMqStatus, bool keybaseStatus,
                     QString mwcMqAddress, int mwcMqAddrIdx) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::Listening),
    state(_state)
{
    ui->setupUi(this);

    //state->setWindowTitle("Listening");

    updateMwcMqState(mwcMqStatus);
    updateKeybaseState(keybaseStatus);
    updateMwcMqAddress(mwcMqAddress, mwcMqAddrIdx);
}

Listening::~Listening()
{
    state->wndIsGone(this);
    delete ui;
}

void Listening::showMessage(QString title, QString message) {
    state::TimeoutLockObject to( state );
    control::MessageBox::message(this, title, message);
}


void Listening::updateMwcMqAddress(QString address, int addrIdx) {

    ui->mwcMqAddress->setText( address );
    ui->mwcMqAddressIndexLabel->setText( addrIdx>=0 ? ("Address Index: " + QString::number(addrIdx)) : "" );
}

void Listening::updateMwcMqState(bool online) {
    ui->mwcMqStatusImg->setPixmap( QPixmap(online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->mwcMqStatusImg->setToolTip(online ? "Listener connected to mwcmq" : "Listener diconnected from mwcmq");
    ui->mwcMqStatusTxt->setText( online ? "Online" : "Offline" );
    ui->mwcMqTriggerButton->setText( online ? "Stop" : "Start" );
    ui->mwcMqTriggerButton->setToolTip(online ? "Stop the MWC MQ Listener" : "Start the MWC MQ Listener");
    ui->mwcMqNextAddress->setEnabled(!online);
    ui->mwcMqToIndex->setEnabled(!online);
}

void Listening::updateKeybaseState(bool online) {

    ui->keybaseStatusImg->setPixmap( QPixmap( online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg" ) );
    ui->keybaseStatusImg->setToolTip(online ? "Listener connected to keybase" : "Listener diconnected from keybase");
    ui->keybaseStatusTxt->setText( online ? "Online" : "Offline" );
    ui->keybaseTriggerButton->setToolTip(online ? "Stop the Keybase Listener" : "Start the Keybase Listener");
    ui->keybaseTriggerButton->setText( online ? "Stop" : "Start" );
}

void Listening::on_mwcMqTriggerButton_clicked()
{
    state->triggerMwcState();
}

void Listening::on_mwcMqNextAddress_clicked()
{
    state->requestNextMwcMqAddress();

}

void Listening::on_mwcMqToIndex_clicked()
{
    state::TimeoutLockObject to( state );
    bool ok = false;
    QString index = control::InputDialog::getText(this, tr("Select MWX box address by index"),
                                            tr("Please specify index of mwc mq address"), "integer from 0 to 65000",
                                            "", 5, &ok);
    if (!ok || index.isEmpty())
        return;

    ok = false;
    int idx = index.toInt(&ok);

    if (!ok || idx<0 || idx>65000) {
        control::MessageBox::message(this, "Wrong value", "Please input integer on the range from 0 to 65000");
        return;
    }

    state->requestNextMwcMqAddressForIndex(idx);
}

void Listening::on_keybaseTriggerButton_clicked()
{
    state->triggerKeybaseState();
}


}




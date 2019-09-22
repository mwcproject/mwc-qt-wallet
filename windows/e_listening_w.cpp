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
#include "../core/Config.h"

namespace wnd {

Listening::Listening(QWidget *parent, state::Listening * _state,
                     const QPair<bool,bool> & listenerStatus, const QPair<bool,bool> & listenerStartState,
                     QString mwcMqAddress, int mwcMqAddrIdx) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::Listening),
    state(_state)
{
    ui->setupUi(this);

    //state->setWindowTitle("Listening");

    updateStatuses( listenerStatus, listenerStartState );

    updateMwcMqAddress(mwcMqAddress, mwcMqAddrIdx);

    ui->mwcMQlable->setText( QString("mwc MQ") + (config::getUseMwcMqS() ? "S" : "") );
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

void Listening::updateStatuses( const QPair<bool,bool> & listenerStatus, const QPair<bool,bool> & listenerStartState ) {
    // MWC MQ
    ui->mwcMqStatusImg->setPixmap(QPixmap(listenerStatus.first ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->mwcMqStatusImg->setToolTip(
            listenerStatus.first ? "Listener connected to mwcmq" : "Listener diconnected from mwcmq");
    ui->mwcMqStatusTxt->setText(listenerStatus.first ? "Online" : "Offline");

    if (listenerStartState.first) {
        if (listenerStatus.first) {
            ui->mwcMqTriggerButton->setText("Stop");
            ui->mwcMqTriggerButton->setToolTip("Stop the MWC MQS Listener");
        } else {
            ui->mwcMqTriggerButton->setText("Stop to retry");
            ui->mwcMqTriggerButton->setToolTip(
                    "MWC MQS Listener already running and trying to reconnect. Click to restart the MWC MQS Listener.");
        }
    } else {
        ui->mwcMqTriggerButton->setText("Start");
        ui->mwcMqTriggerButton->setToolTip("Start the MWC MQS Listener");
    }
    ui->mwcMqNextAddress->setEnabled(!listenerStartState.first);
    ui->mwcMqToIndex->setEnabled(!listenerStartState.first);

    ui->keybaseStatusImg->setPixmap(
            QPixmap(listenerStatus.second ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->keybaseStatusImg->setToolTip(
            listenerStatus.second ? "Listener connected to keybase" : "Listener diconnected from keybase");
    ui->keybaseStatusTxt->setText(listenerStatus.second ? "Online" : "Offline");

    // Keybase
    if (listenerStartState.second) {
        if (listenerStatus.second) {
            ui->keybaseTriggerButton->setText("Stop");
            ui->keybaseTriggerButton->setToolTip("Stop the Keybase Listener");
        } else {
            ui->keybaseTriggerButton->setText("Stop to retry");
            ui->keybaseTriggerButton->setToolTip(
                    "Keybase Listener already running and trying to reconnect. Click to restart the Keybase Listener");
        }
    } else {
        ui->keybaseTriggerButton->setText("Start");
        ui->keybaseTriggerButton->setToolTip("Start the Keybase Listener");
    }
}

void Listening::on_mwcMqTriggerButton_clicked()
{
    state->triggerMwcStartState();
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
        control::MessageBox::message(this, "Wrong value", "Please input integer in the range from 0 to 65000");
        return;
    }

    state->requestNextMwcMqAddressForIndex(idx);
}

void Listening::on_keybaseTriggerButton_clicked()
{
    state->triggerKeybaseStartState();
}


}




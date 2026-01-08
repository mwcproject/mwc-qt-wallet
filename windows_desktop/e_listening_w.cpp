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
#include "../control_desktop/messagebox.h"
#include "../control_desktop/inputdialog.h"
#include <QDebug>
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/x_walletconfig_b.h"
#include "../bridge/heartbeat_b.h"

namespace wnd {

Listening::Listening(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Listening)
{
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    walletConfig = new bridge::WalletConfig(this);
    heartBeat = new bridge::HeartBeat(this);

    QObject::connect( heartBeat, &bridge::HeartBeat::sgnUpdateListenerStatus,
                      this, &Listening::onSgnUpdateListenerStatus, Qt::QueuedConnection);

    updateStatuses();
    setMwcTorAddressWithIndex( wallet->getMqsAddress(), wallet->getTorSlatepackAddress(), wallet->getAddressIndex() );

    ui->mwcMQlable->setText("MWCMQS");

    if (!walletConfig->isFeatureSlatepack()) {
        ui->slatepackFrame->hide();
    }
    if (!walletConfig->isFeatureMWCMQS()) {
        ui->MqsFrame->hide();
    }
    if (!walletConfig->isFeatureTor()) {
        ui->torFrame->hide();
    }
}

Listening::~Listening()
{
    delete ui;
}

// _keybaseOnline is absolete
void Listening::onSgnUpdateListenerStatus(bool mwcOnline, bool tor) {
    Q_UNUSED(mwcOnline);
    Q_UNUSED(tor);

    updateStatuses();
}

void Listening::setMwcTorAddressWithIndex(QString mwcAddress, QString torAddress, int idx) {
    ui->mwcmqsAddress->setText( "mwcmqs://" + mwcAddress );
    ui->mwcmqsAddressIndexLabel->setText( idx>=0 ? ("Address Index: " + QString::number(idx)) : "" );
    ui->torAddressIndexLabel->setText( idx>=0 ? ("Address Index: " + QString::number(idx)) : "" );
    ui->slatepackAddressIndexLabel->setText( idx>=0 ? ("Address Index: " + QString::number(idx)) : "" );

    ui->torAddress->setText( "http://" + torAddress + ".onion" );
    ui->slatepackAddress->setText(torAddress);
}

void Listening::updateStatuses() {

    QVector<bool> statuses = wallet->getListenerStatus();
    Q_ASSERT(statuses.size()==4);
    bool mqsStatus = statuses[1];
    bool torStatus = statuses[3];

    // MWC MQ
    ui->mwcMqStatusImg->setPixmap(QPixmap(mqsStatus ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->mwcMqStatusImg->setToolTip(
            mqsStatus ? "Listener connected to MWCMQS" : "Listener disconnected from MWCMQS");
    ui->mwcMqStatusTxt->setText(mqsStatus ? "Online" : "Offline");

    ui->torStatusImg->setPixmap(
            QPixmap(torStatus ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->torStatusImg->setToolTip(
            torStatus ? "Listener connected to Tor" : "Listener disconnected from Tor");
    ui->torStatusTxt->setText(torStatus ? "Online" : "Offline");
}

static bool warnMsgShown = false;

void Listening::on_mwcMqNextAddress_clicked()
{
    util::TimeoutLockObject to("Listening");

    if (!warnMsgShown && core::WndManager::RETURN_CODE::BTN2 != control::MessageBox::questionText(this, tr("Warning"),
                                      tr("Please note that your wallet will only listen to one address at a time. You are now setting the wallet to listen to a different address.\n\nDo you want to continue?"),
                                      tr("Cancel"), tr("Continue"),
                                      tr("Cancel and keep current wallet address"),
                                      tr("Continue and change my wallet address"),
                                      false, true ))
        return;

    warnMsgShown = true;

    int addressIdx = wallet->getAddressIndex();
    addressIdx = (addressIdx+1) % 65001;
    wallet->setAddressIndex(addressIdx);

    // Updating addresses imediatelly. MQS and tor will restart in the background
    setMwcTorAddressWithIndex( wallet->getMqsAddress(), wallet->getTorSlatepackAddress(), wallet->getAddressIndex() );
}

void Listening::on_mwcMqToIndex_clicked()
{
    util::TimeoutLockObject to("Listening");
    bool ok = false;
    QString index = control::InputDialog::getText(this, tr("Select MWCMQS address by index"),
                                            tr("Please specify the index of the MWCMQS address.\n\nPlease note that your wallet will only listen to one address at a time. You are now setting the wallet to listen to a different address."),
                                            tr("integer from 0 to 65000"),
                                            "", 5, &ok);
    if (!ok || index.isEmpty())
        return;

    ok = false;
    int idx = index.toInt(&ok);

    if (!ok || idx<0 || idx>65000) {
        control::MessageBox::messageText(this, "Wrong value", "Please input integer in the range from 0 to 65000");
        return;
    }

    wallet->setAddressIndex(idx);
    // Updating addresses imediatelly. MQS and tor will restart in the background
    setMwcTorAddressWithIndex( wallet->getMqsAddress(), wallet->getTorSlatepackAddress(), wallet->getAddressIndex() );
}

}

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

namespace wnd {

Listening::Listening(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Listening)
{
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    walletConfig = new bridge::WalletConfig(this);

    QObject::connect( wallet, &bridge::Wallet::sgnUpdateListenerStatus,
                      this, &Listening::onSgnUpdateListenerStatus, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnMwcAddressWithIndex,
                      this, &Listening::onSgnMwcAddressWithIndex, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnListenerStartStop,
                      this, &Listening::onSgnListenerStartStop, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnFileProofAddress,
                      this, &Listening::onSgnFileProofAddress, Qt::QueuedConnection);

    updateStatuses();
    wallet->requestFileProofAddress();
    wallet->requestMqsAddress();

    ui->mwcMQlable->setText("MWC MQS");

    ui->torBridgeConection->setText(config->getTorBridgeConection());
    ui->torBridgeConection->setCursorPosition(0);

    ui->torClientOptions->setText(config->getTorClientOptions());
    ui->torClientOptions->setCursorPosition(0);

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
void Listening::onSgnUpdateListenerStatus(bool mwcOnline, bool _keybaseOnline, bool tor) {
    Q_UNUSED(mwcOnline);
    Q_UNUSED(_keybaseOnline);
    Q_UNUSED(tor);

    updateStatuses();
}
// _keybase is absolete
void Listening::onSgnListenerStartStop(bool mqs, bool tor) {
    Q_UNUSED(mqs);
    Q_UNUSED(tor);
    updateStatuses();
}

void Listening::onSgnMwcAddressWithIndex(QString mwcAddress, int idx) {
    ui->mwcmqsAddress->setText( mwcAddress );
    ui->mwcmqsAddressIndexLabel->setText( idx>=0 ? ("Address Index: " + QString::number(idx)) : "" );
    ui->torAddressIndexLabel->setText( idx>=0 ? ("Address Index: " + QString::number(idx)) : "" );
    ui->slatepackAddressIndexLabel->setText( idx>=0 ? ("Address Index: " + QString::number(idx)) : "" );
}

void Listening::onSgnFileProofAddress(QString proofAddress) // tor address
{
    ui->torAddress->setText( "http://" + proofAddress + ".onion" );
    ui->slatepackAddress->setText(proofAddress);
}

void Listening::updateStatuses() {

    bool mqsStatus = wallet->getMqsListenerStatus();
    bool torStatus = wallet->getTorListenerStatus();

    // MWC MQ
    ui->mwcMqStatusImg->setPixmap(QPixmap(mqsStatus ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->mwcMqStatusImg->setToolTip(
            mqsStatus ? "Listener connected to mwcmqs" : "Listener diconnected from mwcmqs");
    ui->mwcMqStatusTxt->setText(mqsStatus ? "Online" : "Offline");

    ui->torStatusImg->setPixmap(
            QPixmap(torStatus ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->torStatusImg->setToolTip(
            torStatus ? "Listener connected to Tor" : "Listener diconnected from TOR");
    ui->torStatusTxt->setText(torStatus ? "Online" : "Offline");
}

static bool warnMsgShown = false;

void Listening::on_mwcMqNextAddress_clicked()
{
    util::TimeoutLockObject to("Listening");

    if (!warnMsgShown && core::WndManager::RETURN_CODE::BTN2 != control::MessageBox::questionText(this, tr("Warning"),
                                      tr("Please note that your wallet will only listen to one address at a time. You are now setting the wallet to listen to different address.\n\nDo you want to continue?"),
                                      tr("Cancel"), tr("Continue"),
                                      tr("Cancel and keep current wallet address"),
                                      tr("Continue and change my wallet address"),
                                      false, true ))
        return;

    warnMsgShown = true;

    wallet->requestNextMqsAddress();
    wallet->requestFileProofAddress();
}

void Listening::on_mwcMqToIndex_clicked()
{
    util::TimeoutLockObject to("Listening");
    bool ok = false;
    QString index = control::InputDialog::getText(this, tr("Select MQS address by index"),
                                            tr("Please specify index of MWC MQS address.\n\nPlease note that your wallet will only listen to one address at a time. You are now setting the wallet to listen to different address."),
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

    wallet->requestChangeMqsAddress(idx);
    wallet->requestFileProofAddress();
}

void Listening::on_torBridgeConection_textEdited(const QString &)
{
    config->setTorBridgeConectionClientOptions(ui->torBridgeConection->text(), ui->torClientOptions->text());
}

void Listening::on_torClientOptions_textEdited(const QString &)
{
    config->setTorBridgeConectionClientOptions(ui->torBridgeConection->text(), ui->torClientOptions->text());
}


}

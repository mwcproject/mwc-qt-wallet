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
#include "../dialogs_desktop/e_httplistenerconfigdlg.h"
#include "../dialogs_desktop/e_httplistenersimpleconfigdlg.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"

namespace wnd {

Listening::Listening(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Listening)
{
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);

    QObject::connect( wallet, &bridge::Wallet::sgnUpdateListenerStatus,
                      this, &Listening::onSgnUpdateListenerStatus, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnHttpListeningStatus,
                      this, &Listening::onSgnHttpListeningStatus, Qt::QueuedConnection);
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
void Listening::onSgnHttpListeningStatus(bool listening, QString additionalInfo) {
    Q_UNUSED(listening);
    Q_UNUSED(additionalInfo);

    updateStatuses();
}

// _keybase is absolete
void Listening::onSgnListenerStartStop(bool mqs, bool tor) {
    if (mqs)
        mqsInProgress = false;
    if (tor)
        torInProgress = false;

    updateStatuses();
}

void Listening::onSgnMwcAddressWithIndex(QString mwcAddress, int idx) {
    ui->mwcmqsAddress->setText( mwcAddress );
    ui->mwcmqsAddressIndexLabel->setText( idx>=0 ? ("Address Index: " + QString::number(idx)) : "" );
    ui->torAddressIndexLabel->setText( idx>=0 ? ("Address Index: " + QString::number(idx)) : "" );
}

void Listening::onSgnFileProofAddress(QString proofAddress) // tor address
{
    ui->torAddress->setText( "http://" + proofAddress + ".onion" );
}


void Listening::updateStatuses() {

    bool mqsStatus = wallet->getMqsListenerStatus();
    bool torStatus = wallet->getTorListenerStatus();

    bool mqsStarted = wallet->isMqsListenerStarted();
    bool torStarted = wallet->isTorListenerStarted();

    // MWC MQ
    ui->mwcMqStatusImg->setPixmap(QPixmap(mqsStatus ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->mwcMqStatusImg->setToolTip(
            mqsStatus ? "Listener connected to mwcmqs" : "Listener diconnected from mwcmqs");
    ui->mwcMqStatusTxt->setText(mqsStatus ? "Online" : "Offline");

    if (mqsStarted) {
        if (mqsStatus) {
            ui->mwcMqTriggerButton->setText( mqsInProgress ? "Stopping..." : "Stop" );
            ui->mwcMqTriggerButton->setToolTip("Stop the MWC MQS Listener");
        } else {
            ui->mwcMqTriggerButton->setText("Stop to retry");
            ui->mwcMqTriggerButton->setToolTip(
                    "MWC MQS Listener already running and trying to reconnect. Click to restart the MWC MQS Listener.");
        }
    } else {
        ui->mwcMqTriggerButton->setText(mqsInProgress ? "Starting..." : "Start");
        ui->mwcMqTriggerButton->setToolTip("Start the MWC MQS Listener");
    }
    ui->mwcMqNextAddress->setEnabled(!mqsStarted && !torStarted);
    ui->mwcMqToIndex->setEnabled(!mqsStarted && !torStarted);

    ui->torStatusImg->setPixmap(
            QPixmap(torStatus ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->torStatusImg->setToolTip(
            torStatus ? "Listener connected to Tor" : "Listener diconnected from TOR");
    ui->torStatusTxt->setText(torStatus ? "Online" : "Offline");

    // TOR
    if (torStarted) {
        if (torStatus) {
            ui->torTriggerButton->setText(torInProgress ? "Stopping..." : "Stop");
            ui->torTriggerButton->setToolTip("Stop the Tor Listener");
        } else {
            ui->torTriggerButton->setText("Stop to retry");
            ui->torTriggerButton->setToolTip(
                    "Tor Listener is already running and trying to reconnect. Click to restart the Tor Listener");
        }
    } else {
        ui->torTriggerButton->setText( torInProgress ? "Starting" : "Start");
        ui->torTriggerButton->setToolTip("Start the Tor Listener");
    }

    // "true"  - listening
    // ""  - not listening, no errors
    // string  - not listening, error message
    QString httpStatus = wallet->getHttpListeningStatus();
    bool httpOnline = (httpStatus=="true");

    // -------------   HTTP(S)  ------------------
    ui->httpStatusImg->setPixmap( QPixmap( httpOnline ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg" ) );
    ui->httpStatusImg->setToolTip( httpOnline ? "Wallet http(s) foreign REST API is online" : "Wallet foreign REST API is offline");
    ui->httpStatusTxt->setText( httpOnline ? "Online" : "Offline" );

    QString foreignApiAddress = config->getForeignApiAddress();
    bool hasTls = config->hasTls();
    if (hasTls)
        ui->label_http->setText("Https");

    if (httpOnline) {
        QString warningStr;

        if (foreignApiAddress.startsWith("127.0.0.1:")) {
            warningStr += "INFO: Accepting Tor and local connections";
        }
        else {
            if (!hasTls)
                warningStr += "WARNING: You are using non-secure HTTP connection.";
        }

        ui->http_warnings->setText(warningStr);
    } else {
        ui->http_warnings->setText(httpStatus); // String param will an error or nothing if it is disabled. That what we need.
    }
}

void Listening::on_mwcMqTriggerButton_clicked()
{
    if (mqsInProgress)
        return;
    mqsInProgress = true;

    if (wallet->isMqsListenerStarted()) {
        ui->mwcMqTriggerButton->setText("Stopping...");
        wallet->requestStopMqsListener();
    }
    else {
        ui->mwcMqTriggerButton->setText("Starting...");
        wallet->requestStartMqsListener();
    }
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

void Listening::on_torTriggerButton_clicked()
{
    if (torInProgress)
        return;
    torInProgress = true;

    if (wallet->isTorListenerStarted()) {
        ui->torTriggerButton->setText("Stopping...");
        wallet->requestStopTorListener();
    }
    else {
        ui->torTriggerButton->setText("Starting...");
        wallet->requestStartTorListener();
    }
}


void Listening::on_httpConfigButton_clicked()
{
    util::TimeoutLockObject to("Listening");

    // Just start the config dialog. I will take case about itself
//    dlg::HttpListenerConfigDlg optionDlg(this);
    dlg::HttpListenerSimpleConfigDlg optionDlg(this);
    optionDlg.exec();
}


}

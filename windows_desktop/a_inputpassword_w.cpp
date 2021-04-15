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

#include "a_inputpassword_w.h"
#include "ui_a_inputpassword.h"
#include "../control_desktop/messagebox.h"
#include <QThread>
#include <QShortcut>
#include "../util_desktop/widgetutils.h"
#include <QMovie>
#include <QLabel>
#include <QSet>
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/a_inputpassword_b.h"
#include "../bridge/wnd/a_startwallet_b.h"
#include "../bridge/wallet_b.h"
#include "../core_desktop/DesktopWndManager.h"

namespace wnd {

InputPassword::InputPassword(QWidget *parent, bool lockMode) :
        core::PanelBaseWnd(parent),
        ui(new Ui::InputPassword) {
    ui->setupUi(this);

    config = new bridge::Config(this);
    inputPassword = new bridge::InputPassword(this);
    startWallet = new bridge::StartWallet(this);
    wallet = new bridge::Wallet(this);

    QObject::connect(wallet, &bridge::Wallet::sgnUpdateSyncProgress,
                     this, &InputPassword::onSgnUpdateSyncProgress, Qt::QueuedConnection);
    QObject::connect(wallet, &bridge::Wallet::sgnLoginResult,
                     this, &InputPassword::onSgnLoginResult, Qt::QueuedConnection);
    QObject::connect(wallet, &bridge::Wallet::sgnUpdateListenerStatus,
                     this, &InputPassword::onSgnUpdateListenerStatus, Qt::QueuedConnection);
    QObject::connect(wallet, &bridge::Wallet::sgnHttpListeningStatus,
                     this, &InputPassword::onSgnHttpListeningStatus, Qt::QueuedConnection);
    QObject::connect(wallet, &bridge::Wallet::sgnStartingCommand,
                     this, &InputPassword::onSgnStartingCommand, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()));

    if (lockMode) {
        if (config->isOnlineWallet()) {
            updateMwcMqState(wallet->getMqsListenerStatus());
            updateTorState(wallet->getTorListenerStatus());

            QString httpStatus = wallet->getHttpListeningStatus();
            if (config->hasTls()) {
                ui->label_http->setText("Https");
            }
            updateHttpState(httpStatus == "true");
        } else {
            ui->listeningStatusFrameHolder->hide();
        }

        ui->newInstancaHolder->hide();
    } else {
        ui->listeningStatusFrameHolder->hide();
    }

    ui->mwcMQlable->setText("MWC MQS");

    ui->syncStatusMsg->setText("");

    // <selected path_id>, < <path_id>, <instance name>, <network> >, ...  >
    QVector<QString> instanceData = config->getWalletInstances(true);
    // expecting at least 1 instance value
    if (instanceData.size()>=4) {
        // scanning for networks first
        QSet<QString> networkSet;
        for (int i=1; i<=instanceData.size()-3; i+=3) {
            networkSet += instanceData[i+2];
        }
        int selectedIdx = 0;
        for (int i=1; i<=instanceData.size()-3; i+=3) {
            const QString & pathId = instanceData[i];
            const QString & name = instanceData[i+1];
            const QString & nw = instanceData[i+2];

            if (pathId == instanceData[0]) {
                selectedIdx = i/3;
            }
            ui->accountComboBox->insertItem(i/3, networkSet.size()>1 ? name + "; " + nw : name, QVariant(pathId));
        }
        ui->accountComboBox->setCurrentIndex(selectedIdx);
    }
}

void InputPassword::panelWndStarted() {
    ui->passwordEdit->setFocus();
}


InputPassword::~InputPassword() {
    delete ui;
}

void InputPassword::on_submitButton_clicked() {
    util::TimeoutLockObject to("InputPassword");

    QString pswd = ui->passwordEdit->text();

    if (pswd.isEmpty()) {
        control::MessageBox::messageText(this, "Password", "Please input your wallet password");
        return;
    }

    QPair<bool, QString> valRes = util::validateMwc713Str(pswd, true);
    if (!valRes.first) {
        control::MessageBox::messageText(this, "Password", valRes.second);
        return;
    }

    if (pswd.startsWith("-")) {
        control::MessageBox::messageText(this, "Password", "You can't start your password from '-' symbol.");
        return;
    }

    QString selectedPath = ui->accountComboBox->currentData().toString();
    config->setActiveInstance(selectedPath);

    ui->progress->show();

    // Submit the password and wait until state will push us.
    inputPassword->submitPassword(pswd);
    // Because of event driven, the flow is not linear
}


void InputPassword::onSgnLoginResult(bool ok) {
    if (!ok) {
        ui->progress->hide();

        util::TimeoutLockObject to("InputPassword");
        control::MessageBox::messageText(this, "Password",
                                         "Password supplied was incorrect. Please input correct password.");

        QThread::sleep(1); // sleep to prevent brute force attack.
        // Note, we are using small hash, so the brute force attach will likely
        // found wong password with similar hash.

        ui->passwordEdit->setText("");
        ui->passwordEdit->setFocus(Qt::OtherFocusReason);
    }

    ui->passwordEdit->setEnabled(!ok);
    ui->submitButton->setEnabled(!ok);
    ui->restoreInstanceButton->setEnabled(!ok);
    ui->newInstanceButton->setEnabled(!ok);
    ui->openWalletButton->setEnabled(!ok);
}

void InputPassword::updateMwcMqState(bool online) {
    ui->mwcMqStatusImg->setPixmap(QPixmap(online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->mwcMqStatusImg->setToolTip(online ? "Listener connected to mwcmq" : "Listener diconnected from mwcmq");
    ui->mwcMqStatusTxt->setText(online ? "Online" : "Offline");
}

void InputPassword::updateTorState(bool online) {
    ui->torStatusImg->setPixmap(QPixmap(online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->torStatusImg->setToolTip(online ? "Listener connected to TOR" : "Listener diconnected from TOR");
    ui->torStatusTxt->setText(online ? "Online" : "Offline");
}

void InputPassword::updateHttpState(bool online) {
    ui->httpStatusImg->setPixmap(QPixmap(online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg"));
    ui->httpStatusImg->setToolTip(
            online ? "Wallet http(s) foreign REST API is online" : "Wallet foreign REST API is offline");
    ui->httpStatusTxt->setText(online ? "Online" : "Offline");
}

void InputPassword::onSgnUpdateListenerStatus(bool mwcOnline, bool tor) {
    updateMwcMqState(mwcOnline);
    updateTorState(tor);
}

void InputPassword::onSgnHttpListeningStatus(bool listening, QString additionalInfo) {
    Q_UNUSED(additionalInfo);
    updateHttpState(listening);
}

void InputPassword::onSgnUpdateSyncProgress(double progressPercent) {
    ui->syncStatusMsg->setText(
            "Wallet state update, " + util::trimStrAsDouble(QString::number(progressPercent), 4) + "% complete");
}

void InputPassword::on_restoreInstanceButton_clicked() {
    startWallet->createNewWalletInstance("", true);
}

void InputPassword::on_newInstanceButton_clicked() {
    startWallet->createNewWalletInstance("", false);
}

void InputPassword::on_openWalletButton_clicked()
{
    QString wallet_dir = core::selectWalletDirectory();
    if (wallet_dir.isEmpty())
         return;
    startWallet->createNewWalletInstance(wallet_dir, false);
}

void InputPassword::onSgnStartingCommand(QString actionName) {
    ui->syncStatusMsg->setText(actionName);
}

}


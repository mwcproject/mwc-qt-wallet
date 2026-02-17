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
#include "../bridge/wnd/x_walletconfig_b.h"
#include "../bridge/wnd/a_inputpassword_b.h"
#include "../bridge/wnd/a_startwallet_b.h"
#include "../bridge/wallet_b.h"
#include "../core_desktop/DesktopWndManager.h"
#include "bridge/heartbeat_b.h"
#include <QJsonArray>

namespace wnd {

InputPassword::InputPassword(QWidget *parent, bool lockMode) :
        core::PanelBaseWnd(parent),
        ui(new Ui::InputPassword) {
    ui->setupUi(this);

    config = new bridge::Config(this);
    walletConfig = new bridge::WalletConfig(this);
    inputPassword = new bridge::InputPassword(this);
    startWallet = new bridge::StartWallet(this);
    wallet = new bridge::Wallet(this);
    heartBeat = new bridge::HeartBeat(this);

    QObject::connect(wallet, &bridge::Wallet::sgnScanProgress,
                     this, &InputPassword::onSgnScanProgress, Qt::QueuedConnection);
    QObject::connect(wallet, &bridge::Wallet::sgnScanDone,
                 this, &InputPassword::onSgnScanDone, Qt::QueuedConnection);
    QObject::connect(heartBeat, &bridge::HeartBeat::sgnUpdateListenerStatus,
                     this, &InputPassword::onSgnUpdateListenerStatus, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()));

    if (lockMode) {
        if (config->isOnlineWallet()) {
            QVector<bool> res = wallet->getListenerStatus();

            updateMwcMqState( res[1]);
            updateTorState(res[3]);

            if (!walletConfig->isFeatureMWCMQS()) {
                ui->mqsFrame->hide();
                ui->listeningStatusFrame->layout()->removeItem(ui->middleSpacer);
            }
            if (!walletConfig->isFeatureTor()) {
                ui->torFrame->hide();
                ui->listeningStatusFrame->layout()->removeItem(ui->middleSpacer);
            }
        } else {
            ui->listeningStatusFrameHolder->hide();
        }

        ui->newInstancaHolder->hide();
    } else {
        ui->listeningStatusFrameHolder->hide();
    }

    ui->mwcMQlable->setText("MWCMQS");

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

    // Submit the password and wait until state will push us.
    bool ok = inputPassword->submitPassword(pswd, selectedPath);
    if (!ok) {
        ui->progress->hide();
        ui->passwordEdit->setText("");
        ui->passwordEdit->setFocus(Qt::OtherFocusReason);

        QThread::msleep(300); // sleep to prevent brute force attack.
    }

    ui->passwordEdit->setEnabled(!ok);
    ui->submitButton->setEnabled(!ok);
    ui->restoreInstanceButton->setEnabled(!ok);
    ui->newInstanceButton->setEnabled(!ok);
    ui->openWalletButton->setEnabled(!ok);
}


void InputPassword::updateMwcMqState(bool online) {
    ui->mwcMqStatusImg->setPixmap(QPixmap(online ? ":/img/StatusOk.svg" : ":/img/StatusEmpty.svg"));
    ui->mwcMqStatusImg->setToolTip(online ? "Listener connected to MWCMQS" : "Listener disconnected from MWCMQS");
    ui->mwcMqStatusTxt->setText(online ? "Online" : "Offline");
}

void InputPassword::updateTorState(bool online) {
    ui->torStatusImg->setPixmap(QPixmap(online ? ":/img/StatusOk.svg" : ":/img/StatusEmpty.svg"));
    ui->torStatusImg->setToolTip(online ? "Listener connected to Tor" : "Listener disconnected from Tor");
    ui->torStatusTxt->setText(online ? "Online" : "Offline");
}

void InputPassword::onSgnUpdateListenerStatus(bool mwcOnline, bool tor) {
    updateMwcMqState(mwcOnline);
    updateTorState(tor);
}

// Show updates for any ID.
void InputPassword::onSgnScanProgress(QString responseId, QJsonObject statusMessage) {
    Q_UNUSED(responseId)
    if (statusMessage.contains("Scanning")) {
        QJsonArray vals = statusMessage["Scanning"].toArray();
        int percent_progress = vals[2].toInt();
        ui->syncStatusMsg->setText("Wallet state update, " + QString::number(percent_progress) + "% complete");
    }
}

void InputPassword::onSgnScanDone( QString responseId, bool fullScan, int height, QString errorMessage ) {
    Q_UNUSED(responseId)
    Q_UNUSED(fullScan)
    Q_UNUSED(height)
    Q_UNUSED(errorMessage)

    ui->syncStatusMsg->setText("");
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


}

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
#include "../state/a_inputpassword.h"
#include "../control/messagebox.h"
#include <QThread>
#include <QShortcut>
#include "../util/widgetutils.h"
#include "../util/stringutils.h"
#include <QMovie>
#include <QLabel>
#include "../core/global.h"
#include "../state/timeoutlock.h"
#include "../dialogs/x_walletinstances.h"
#include "../core/Config.h"

namespace wnd {

InputPassword::InputPassword(QWidget *parent, state::InputPassword * _state, state::WalletConfig * _configState, bool lockMode) :
    QWidget(parent),
    ui(new Ui::InputPassword),
    state(_state),
    configState(_configState)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    setFocusPolicy(Qt::StrongFocus);
    ui->passwordEdit->setFocus(Qt::OtherFocusReason);

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );

    if (lockMode) {
        QPair<bool,bool> status = state->getWalletListeningStatus();

        updateMwcMqState(status.first);
        updateKeybaseState(status.second);

        ui->instancesButton->hide();
    }
    else {
        ui->listeningStatusFrame->hide();
    }

    ui->mwcMQlable->setText( QString("mwc MQ") + (config::getUseMwcMqS() ? "S" : "") );
}

InputPassword::~InputPassword()
{
    state->deleteWnd(this);

    delete ui;
}

void InputPassword::on_submitButton_clicked() {
    state::TimeoutLockObject to(state);

    QString pswd = ui->passwordEdit->text();

    QPair <bool, QString> valRes = util::validateMwc713Str(pswd, true);
    if (!valRes.first) {
        control::MessageBox::message(this, "Password", valRes.second );
        return;
    }

    // Submit the password and wait until state will push us.
    state->submitPassword(pswd);
    // Because of event driven, the flow is not linear
}

void InputPassword::startWaiting() {
    ui->progress->show();
}

void InputPassword::stopWaiting() {
    ui->progress->hide();
}

void InputPassword::reportWrongPassword() {
    state::TimeoutLockObject to(state);
    control::MessageBox::message(this, "Password", "Password supplied was incorrect. Please input correct password.");

    QThread::sleep(1); // sleep to prevent brute force attack.
    // Note, we are using small hash, so the brute force attach will likely
    // found wong password with similar hash.

    ui->passwordEdit->setText("");
    ui->passwordEdit->setFocus(Qt::OtherFocusReason);
}

void InputPassword::on_instancesButton_clicked()
{
    state::TimeoutLockObject to(state);

    dlg::WalletInstances  walletInstances(this, configState);
    walletInstances.exec();
}

void InputPassword::updateMwcMqState(bool online) {
    ui->mwcMqStatusImg->setPixmap( QPixmap(online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->mwcMqStatusImg->setToolTip(online ? "Listener connected to mwcmq" : "Listener diconnected from mwcmq");
    ui->mwcMqStatusTxt->setText( online ? "Online" : "Offline" );
}

void InputPassword::updateKeybaseState(bool online) {

    ui->keybaseStatusImg->setPixmap( QPixmap( online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg" ) );
    ui->keybaseStatusImg->setToolTip(online ? "Listener connected to keybase" : "Listener diconnected from keybase");
    ui->keybaseStatusTxt->setText( online ? "Online" : "Offline" );
}


}



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

#include "a_initaccount_w.h"
#include "ui_a_initaccount.h"
#include "../util/passwordanalyser.h"
#include "../state/a_initaccount.h"
#include "../util_desktop/widgetutils.h"
#include "../control_desktop/messagebox.h"
#include <QShortcut>
#include <QKeyEvent>
#include "../util_desktop/timeoutlock.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/a_startwallet_b.h"
#include "../bridge/wnd/a_initaccount_b.h"
#include "../bridge/wnd/y_selectmode_b.h"
#include "../core/Config.h"
#include "../core_desktop/DesktopWndManager.h"

namespace wnd {

InitAccount::InitAccount(QWidget *parent, QString path, bool _restoredFromSeed) :
        core::PanelBaseWnd(parent),
        ui(new Ui::InitAccount),
        restoredFromSeed(_restoredFromSeed) {
    ui->setupUi(this);

    selectMode = new bridge::SelectMode(this);
    initAccount = new bridge::InitAccount(this);
    util = new bridge::Util(this);
    startWallet = new bridge::StartWallet(this);

    util->passwordQualitySet(ui->password1Edit->text());
    ui->strengthLabel->setText("");
    ui->submitButton->setEnabled(util->passwordQualityIsAcceptable());

    ui->password1Edit->installEventFilter(this);

    // Default is allways a mainnet
    ui->radioMainNet->setChecked(true);

    ui->createdDirLabel->setText("You are saving your data to:\n" + path);

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()));
    updatePassState();

    ui->straighHolder->hide();

    ui->passwordWndHolder->adjustSize();

    startTimer(200); // 1 second timer is fine. Timer is for try.
}

void InitAccount::panelWndStarted() {
    ui->password1Edit->setFocus();
}


InitAccount::~InitAccount() {
    util->releasePasswordAnalyser();
    delete ui;
}

void InitAccount::on_password1Edit_textChanged(const QString &text) {
    QPair<bool, QString> valRes = util::validateMwc713Str(text, true);
    if (!valRes.first) {
        ui->strengthLabel->setText( text.isEmpty() ? "" : valRes.second);
        ui->strengthLabel->setStyleSheet("background-color: #CCFF33; color: #3600C9");
        ui->submitButton->setEnabled(false);
        ui->submitButton->setEnabled(util->passwordQualityIsAcceptable());
    }
    else {
        util->passwordQualitySet(text);
        ui->strengthLabel->setText(util->passwordQualityComment());

        if (!util->passwordQualityIsAcceptable())
            ui->strengthLabel->setStyleSheet("background-color: #CCFF33");
        else
            ui->strengthLabel->setStyleSheet("");

        ui->submitButton->setEnabled(util->passwordQualityIsAcceptable());
    }
    updatePassState();

    // Hiding only. Show will be on the timer...
    if (ui->strengthLabel->text().isEmpty()) {
        ui->straighHolder->hide();
        warningCounters = 0;
    }

    ui->passwordWndHolder->adjustSize();
}

void InitAccount::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event)

    QString passWarnText = ui->strengthLabel->text();
    if (passWarnText.isEmpty())
        return;

     passWarnText += "|" + ui->password1Edit->text();

    if (!ui->straighHolder->isHidden()) {
        return;
    }

    if (passWarnText == lastWarningStr) {
        warningCounters++;
    }
    else {
        lastWarningStr = passWarnText;
        warningCounters = 0;
    }

    if (warningCounters==5) {
        ui->straighHolder->show();
        ui->passwordWndHolder->adjustSize();
    }
}


void InitAccount::on_password2Edit_textChanged(const QString &text) {
    Q_UNUSED(text)
    updatePassState();
}

void InitAccount::updatePassState() {
    QString pswd1 = ui->password1Edit->text();
    QString pswd2 = ui->password2Edit->text();

    if (pswd2.isEmpty()) {
        ui->confirmPassLable->hide();
    } else {
        ui->confirmPassLable->show();
        ui->confirmPassLable->setPixmap(QPixmap(pswd1 == pswd2 ? ":/img/PassOK@2x.svg" : ":/img/PassNotMatch@2x.svg"));
    }
}


void InitAccount::on_submitButton_clicked() {
    util::TimeoutLockObject to("InitAccount");

    QString instanceName = ui->instanceNameEdit->text();
    if (instanceName.isEmpty()) {
        control::MessageBox::messageText(this, "Instance name", "Please define your wallet instance name.");
        ui->instanceNameEdit->setFocus();
        return;
    }

    QString pswd1 = ui->password1Edit->text();
    QString pswd2 = ui->password2Edit->text();

    QPair<bool, QString> valRes = util::validateMwc713Str(pswd1, true);
    if (!valRes.first) {
        control::MessageBox::messageText(this, "Password", valRes.second);
        return;
    }

    util->passwordQualitySet(pswd1);

    if (!util->passwordQualityIsAcceptable())
    if (!util->passwordQualityIsAcceptable())
        return;

    if (pswd1 != pswd2) {
        control::MessageBox::messageText(this, "Password",
                                         "Password doesn't match confirm string. Please retype the password correctly");
        return;
    }

    util->releasePasswordAnalyser();

    initAccount->setPassword(pswd1);
    initAccount->submitWalletCreateChoices(
            ui->radioMainNet->isChecked() ? state::InitAccount::MWC_NETWORK::MWC_MAIN_NET
                                          : state::InitAccount::MWC_NETWORK::MWC_FLOO_NET,
            instanceName);
}

void wnd::InitAccount::on_runOnlineNodeButton_clicked() {
    util::TimeoutLockObject to("InitAccount");
    if (core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionText(this, "Running Mode",
                                                                                 "You are switching to 'Online Node'.\nOnline Node can be used as a data provider for the Cold Wallet.",
                                                                                 "Cancel", "Continue",
                                                                                 "Don't switch to Online Node, keep my wallet as it is",
                                                                                 "Continue and restart as Online Node",
                                                                                 false, true)) {
        // Restarting wallet in a right mode...
        // First, let's upadte a config
        selectMode->updateWalletRunMode(int(config::WALLET_RUN_MODE::ONLINE_NODE));
    }
}

void InitAccount::on_changeDirButton_clicked() {
    QString wallet_dir = core::selectWalletDirectory();
    if (wallet_dir.isEmpty())
        return;
    startWallet->createNewWalletInstance(wallet_dir, restoredFromSeed);
    ui->createdDirLabel->setText("You are saving your data to:\n" + wallet_dir);
}

}

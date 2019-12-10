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
#include "../util/widgetutils.h"
#include "../control/messagebox.h"
#include <QShortcut>
#include <QKeyEvent>
#include "../core/global.h"
#include "../util/stringutils.h"
#include "../state/timeoutlock.h"
#include "../dialogs/x_walletinstances.h"
#include "../state/y_selectmode.h"
#include "../core/Config.h"

namespace wnd {

InitAccount::InitAccount(QWidget *parent, state::InitAccount * _state, state::WalletConfig * _configState) :
    QWidget(parent),
    ui(new Ui::InitAccount),
    state(_state),
    configState(_configState)
{
    ui->setupUi(this);

    QVector<double> weight;
    QStringList seqWords, dictWords;
    QPair<QString, bool> paResp = passwordAnalyser.getPasswordQualityReport( ui->password1Edit->text(), weight, seqWords, dictWords );

    ui->strengthLabel->setText( paResp.first );
    ui->submitButton->setEnabled( paResp.second );

    ui->password1Edit->installEventFilter(this);

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
    updatePassState();
}

InitAccount::~InitAccount()
{
    delete ui;
}

void InitAccount::on_password1Edit_textChanged(const QString &text)
{
    QPair <bool, QString> valRes = util::validateMwc713Str(text, true);
    if (!valRes.first) {
        ui->strengthLabel->setText( valRes.second );
        ui->submitButton->setEnabled( false );
    }

    QVector<double> weight;
    QStringList seqWords, dictWords;
    QPair<QString, bool> paResp = passwordAnalyser.getPasswordQualityReport( text, weight, seqWords, dictWords );

    ui->strengthLabel->setText(paResp.first);
    ui->submitButton->setEnabled( paResp.second );

    updatePassState();
}

void InitAccount::on_password2Edit_textChanged( const QString &text )
{
    Q_UNUSED(text)
    updatePassState();
}

void InitAccount::updatePassState() {
    QString pswd1 = ui->password1Edit->text();
    QString pswd2 = ui->password2Edit->text();

    if (pswd2.isEmpty()) {
        ui->confirmPassLable->hide();
    }
    else {
        ui->confirmPassLable->show();
        ui->confirmPassLable->setPixmap( QPixmap( pswd1==pswd2 ? ":/img/PassOK@2x.svg" : ":/img/PassNotMatch@2x.svg") );
    }
}



void InitAccount::on_submitButton_clicked()
{
    state::TimeoutLockObject to(state);

    QString pswd1 = ui->password1Edit->text();
    QString pswd2 = ui->password2Edit->text();

    QPair <bool, QString> valRes = util::validateMwc713Str(pswd1, true);
    if (!valRes.first) {
        control::MessageBox::messageText(this, "Password", valRes.second );
        return;
    }

    QVector<double> weight;
    QStringList seqWords, dictWords;
    QPair<QString, bool> paResp = passwordAnalyser.getPasswordQualityReport( pswd1, weight, seqWords, dictWords );

    if (!paResp.second)
        return;

    if (pswd1!=pswd2) {
        control::MessageBox::messageText(this, "Password", "Password doesn't match confirm string. Please retype the password correctly");
        return;
    }

    if (! paResp.second ) {
        control::MessageBox::messageText(this, "Password", "Your password is not strong enough. Please input stronger password");
        return;
    }

    state->setPassword(pswd1);
}

void InitAccount::on_instancesButton_clicked()
{
    state::TimeoutLockObject to(state);

    dlg::WalletInstances  walletInstances(this, configState);
    walletInstances.exec();
}

void wnd::InitAccount::on_runOnlineNodeButton_clicked()
{
    state::TimeoutLockObject to(state);
    if ( control::MessageBox::RETURN_CODE::BTN2 == control::MessageBox::questionText(this, "Running Mode",
                          "You are switching to 'Online Node'.\nOnline Node can be used as a data provider for Cols Wallet.",
                                  "Cancel", "Continue", false, true) ) {
        // Restarting wallet in a right mode...
        // First, let's upadte a config
        state::SelectMode::updateWalletRunMode( config::WALLET_RUN_MODE::ONLINE_NODE );
    }
}

}



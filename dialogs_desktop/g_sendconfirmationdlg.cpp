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

#include "g_sendconfirmationdlg.h"
#include "ui_g_sendconfirmationdlg.h"
#include <Qt>
#include <QTextDocument>
#include <QScreen>
#include <QTextBlock>
#include <QScrollBar>
#include <QThread>
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"
#include "../util_desktop/widgetutils.h"

namespace dlg {

SendConfirmationDlg::SendConfirmationDlg( QWidget *parent, QString title, QString message, double widthScale, QString passwordHash ) :
     MwcDialog(parent),
    ui(new Ui::SendConfirmationDlg),
    blockingPasswordHash(passwordHash)
{
    ui->setupUi(this);
    util = new bridge::Util(this);
    config = new bridge::Config(this);

    if (widthScale!=1.0) {
        // Let's ujust Width first
        int w = maximumWidth();
        w = int(w*widthScale);
        setMaximumWidth(w);

        QSize sz = size();
        sz.setWidth( sz.width()*widthScale );
        resize(sz);
        setMinimumWidth(sz.width());

        adjustSize();
    }

    ui->title->setText(title);

    utils::resizeEditByContent(this, ui->text3, false, message);

    ui->fluffCheckBox->setCheckState( config->isFluffSet() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

    ui->confirmButton->setEnabled(false);

    ui->declineButton->setFocus();
    ui->confirmButton->setFocus();

    ui->confirmButton->adjustSize();
    ui->declineButton->adjustSize();

    adjustSize();

    if (!blockingPasswordHash.isEmpty()) {
        ui->passwordEdit->setFocus();
    }
}

SendConfirmationDlg::~SendConfirmationDlg()
{
    delete ui;
}


void SendConfirmationDlg::on_passwordEdit_textChanged(const QString &str)
{
    QThread::msleep(200); // Ok for human and will prevent brute force from UI attack (really crasy scenario, better to attack mwc713 if you already get the host).
    bool ok = util->calcHSA256Hash(str) == blockingPasswordHash;
    ui->confirmButton->setEnabled(ok);
    if (ok)
        ui->confirmButton->setFocus();
}

void SendConfirmationDlg::on_declineButton_clicked()
{
    reject();
}

void SendConfirmationDlg::on_confirmButton_clicked()
{
    config->setFluff(ui->fluffCheckBox->isChecked());
    accept();
}


}


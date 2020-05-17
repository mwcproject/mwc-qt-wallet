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

#include "dialogs/g_sendconfirmationdlg.h"
#include "ui_g_sendconfirmationdlg.h"
#include <Qt>
#include <QTextDocument>
#include <QScreen>
#include <QTextBlock>
#include <QScrollBar>
#include <QThread>
#include "../util/crypto.h"

namespace dlg {

SendConfirmationDlg::SendConfirmationDlg( QWidget *parent, QString title, QString message, double widthScale, QString passwordHash, bool fluffTxn ) :
     MwcDialog(parent),
    ui(new Ui::SendConfirmationDlg),
    blockingPasswordHash(passwordHash),
    origFluffSetting(fluffTxn),
    newFluffSetting(fluffTxn)
{
    ui->setupUi(this);

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

    // Setting text option
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textOption.setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    ui->text3->document()->setDefaultTextOption( textOption );

    // prepare for rendering
    QSize curSz = ui->text3->size();

    // text as a data should work for as.
    ui->text3->setPlainText(message);

    // size is the wierdest part. We are renderind document to get a size form it.
    // Document tolk in Pt, so need to convert into px. Conversion is not very accurate
    ui->text3->document()->adjustSize();
    int h = int(curSz.height());
    ui->text3->adjustSize();

    // Second Ajustment with a scroll br that works great
    QScrollBar * vSb = ui->text3->verticalScrollBar();
    int scrollDiff = vSb->maximum() - vSb->minimum();
    int page = vSb->pageStep();

    if (scrollDiff >0) {
        h = int( h * double(scrollDiff + page)/page + 1);
    }
    ui->text3->setMaximumHeight( h );
    ui->text3->setMinimumHeight( h );
    ui->text3->adjustSize();

    if (fluffTxn) {
        ui->fluffCheckBox->setCheckState(Qt::CheckState::Checked);
    }


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
    ui->confirmButton->setEnabled( crypto::calcHSA256Hash(str) == blockingPasswordHash);
}

void SendConfirmationDlg::on_declineButton_clicked()
{
    newFluffSetting = origFluffSetting;
    retCode = RETURN_CODE::DECLINE;
    accept();
}

void SendConfirmationDlg::on_confirmButton_clicked()
{
    newFluffSetting = ui->fluffCheckBox->isChecked();
    if (origFluffSetting != newFluffSetting) {
        emit saveFluffSetting(newFluffSetting);
    }
    retCode = RETURN_CODE::CONFIRM;
    accept();
}


}


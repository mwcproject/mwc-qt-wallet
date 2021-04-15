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

#include "messagebox.h"
#include "ui_messagebox.h"
#include <Qt>
#include <QDebug>
#include <QTextDocument>
#include <QScreen>
#include <QTextBlock>
#include <QScrollBar>
#include <QThread>
#include "../util/crypto.h"
#include "../util_desktop/widgetutils.h"
#include "MwcLabelProgress.h"

namespace control {

// Password accepted as a HASH. EMpty String mean that no password is set.
// After return, passwordHash value will have input raw Password value. So it can be user for wallet
MessageBox::MessageBox( QWidget *parent, QString title, QString message, bool htmlMsg, QString btn1, QString btn2,
        QString btn1Tooltip, QString btn2Tooltip,
        bool default1, bool default2, double widthScale, QString & passwordHash, core::WndManager::RETURN_CODE _passBlockButton,
        int ttl_bls ) :
     MwcDialog(parent),
    ui(new Ui::MessageBox),
    blockingPasswordHash(passwordHash),
    passBlockButton(_passBlockButton),
    ttl_blocks(ttl_bls)
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

    utils::resizeEditByContent(this, ui->text3, htmlMsg, message);


    if (blockingPasswordHash.isEmpty()) {
        ui->passwordFrame->hide();
    }
    else {
        // disable blocking button
        if (passBlockButton == core::WndManager::RETURN_CODE::BTN1)
            ui->button1->setEnabled(false);
        else
            ui->button2->setEnabled(false);
    }

    if (ttl_blocks < 0) {
        ui->inputTTLBlocksLabel->hide();
        ui->TTLEdit->hide();
    }
    else {
        ui->TTLEdit->setText(QString::number(ttl_blocks));
    }


    if (btn1.isEmpty()) {
        ui->button1->hide();
        QLayoutItem * cntL = ui->central_HorizontalSpacer->layout()->takeAt(2);
        delete cntL;
        ui->central_HorizontalSpacer = nullptr;
    }
    else {
        ui->button1->setText(btn1);
        if (default1)
            ui->button1->setFocus();
    }

    if (btn2.isEmpty()) {
        ui->button2->hide();
        if (ui->central_HorizontalSpacer) {
            QLayoutItem * cntL = ui->horizontalLayout->takeAt(2);
            delete cntL;
            ui->central_HorizontalSpacer = nullptr;
        }
    }
    else {
        ui->button2->setText(btn2);
        if (default2)
            ui->button2->setFocus();
    }

    ui->button2->adjustSize();
    ui->button1->adjustSize();

    if (!btn1Tooltip.isEmpty())
        ui->button1->setToolTip(btn1Tooltip);

    if (!btn2Tooltip.isEmpty())
        ui->button2->setToolTip(btn2Tooltip);

    adjustSize();

    if (!blockingPasswordHash.isEmpty()) {
        ui->passwordEdit->setFocus();
    }

    onMessageBoxShown();
}

MessageBox::~MessageBox()
{
    delete ui;
}


void MessageBox::on_passwordEdit_textChanged(const QString &str)
{
    control::MwcPushButtonNormal * btn2lock = passBlockButton == core::WndManager::RETURN_CODE::BTN1 ? ui->button1 : ui->button2;
    bool ok = crypto::calcHSA256Hash(str) == blockingPasswordHash;
    btn2lock->setEnabled(ok);
    if (ok)
        btn2lock->setFocus();
}

void MessageBox::on_button1_clicked()
{
    processApplyButton(core::WndManager::RETURN_CODE::BTN1);
}

void MessageBox::on_button2_clicked()
{
    processApplyButton(core::WndManager::RETURN_CODE::BTN2);
}

void MessageBox::processApplyButton(core::WndManager::RETURN_CODE rc) {
    retCode = rc;
    blockingPasswordHash = ui->passwordEdit->text();
    if (ttl_blocks>=0) {
        int blks = ui->TTLEdit->text().toInt();
        if (blks < 10) {
            MessageBox::messageText(this, "TTL value",
                                    "Please specify valid TTL value. Minimum valid TTL value is 10.");
            return;
        }
        ttl_blocks = blks;
    }
    accept();
}


// One button, OK box
//static
void MessageBox::messageText( QWidget *parent, QString title, QString message, double widthScale ) {
    QString hash;
    MessageBox * msgBox = new MessageBox(parent, title, message, false, "OK", "", "","", true,false, widthScale, hash, core::WndManager::RETURN_CODE::BTN1, -1 );
    msgBox->exec();
    delete msgBox;
}

void MessageBox::messageHTML( QWidget *parent, QString title, QString message, double widthScale ) {
    QString hash;
    MessageBox * msgBox = new MessageBox(parent, title, message, true, "OK", "", "","", true,false, widthScale, hash, core::WndManager::RETURN_CODE::BTN1, -1 );
    msgBox->exec();
    delete msgBox;
}

// Two button box
//static
core::WndManager::RETURN_CODE MessageBox::questionText( QWidget *parent, QString title, QString message, QString btn1, QString btn2,
                   QString btn1Tooltip, QString btn2Tooltip,
                   bool default1, bool default2, double widthScale, QString & passwordHash, core::WndManager::RETURN_CODE blockButton ) {
    MessageBox * msgBox = new MessageBox(parent, title, message, false, btn1, btn2, btn1Tooltip, btn2Tooltip, default1, default2, widthScale, passwordHash, blockButton, -1 );
    msgBox->exec();
    core::WndManager::RETURN_CODE  res = msgBox->getRetCode();
    delete msgBox;
    return res;
}

core::WndManager::RETURN_CODE MessageBox::questionTextWithTTL( QWidget *parent, QString title, QString message, QString btn1, QString btn2,
                   QString btn1Tooltip, QString btn2Tooltip,
                   bool default1, bool default2, double widthScale, QString & passwordHash, core::WndManager::RETURN_CODE blockButton, int *ttl_blocks ) {
    MessageBox * msgBox = new MessageBox(parent, title, message, false, btn1, btn2, btn1Tooltip, btn2Tooltip,
                                         default1, default2, widthScale, passwordHash, blockButton,
                                         ttl_blocks == nullptr ? -1 : *ttl_blocks );
    msgBox->exec();
    if ( ttl_blocks != nullptr)
        *ttl_blocks = msgBox->ttl_blocks;
    core::WndManager::RETURN_CODE  res = msgBox->getRetCode();
    delete msgBox;
    return res;
}

core::WndManager::RETURN_CODE MessageBox::questionText( QWidget *parent, QString title, QString message, QString btn1, QString btn2,
        QString btn1Tooltip, QString btn2Tooltip,
        bool default1, bool default2, double widthScale) {
    QString hash;
    MessageBox * msgBox = new MessageBox(parent, title, message, false, btn1, btn2, btn1Tooltip, btn2Tooltip, default1, default2, widthScale, hash, core::WndManager::RETURN_CODE::BTN1, -1);
    msgBox->exec();
    core::WndManager::RETURN_CODE  res = msgBox->getRetCode();
    delete msgBox;
    return res;
}
//questionTextWithTTL(QWidget*, QString, QString, QString, QString, QString, QString, bool, bool, double, int*)
core::WndManager::RETURN_CODE MessageBox::questionTextWithTTL( QWidget *parent, QString title, QString message, QString btn1, QString btn2,
        QString btn1Tooltip, QString btn2Tooltip,
        bool default1, bool default2, double widthScale, int *ttl_blocks) {
    QString hash;
    MessageBox * msgBox = new MessageBox(parent, title, message, false, btn1, btn2, btn1Tooltip, btn2Tooltip,
                                         default1, default2, widthScale, hash, core::WndManager::RETURN_CODE::BTN1,
                                         ttl_blocks== nullptr ? -1 : *ttl_blocks);
    msgBox->exec();
    if ( ttl_blocks != nullptr )
    	*ttl_blocks = msgBox->ttl_blocks;
    core::WndManager::RETURN_CODE  res = msgBox->getRetCode();
    delete msgBox;
    return res;
}

core::WndManager::RETURN_CODE MessageBox::questionHTML( QWidget *parent, QString title, QString message, QString btn1, QString btn2,
            QString btn1Tooltip, QString btn2Tooltip,
            bool default1, bool default2, double widthScale ) {
    QString hash;
    MessageBox * msgBox = new MessageBox(parent, title, message, true, btn1, btn2, btn1Tooltip, btn2Tooltip, default1, default2, widthScale, hash, core::WndManager::RETURN_CODE::BTN1, -1);
    msgBox->exec();
    core::WndManager::RETURN_CODE  res = msgBox->getRetCode();
    delete msgBox;
    return res;
}



}


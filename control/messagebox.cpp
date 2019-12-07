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

#include "control/messagebox.h"
#include "ui_messagebox.h"
#include <Qt>
#include <QTextDocument>
#include <QScreen>
#include <QTextBlock>
#include <QScrollBar>

namespace control {

MessageBox::MessageBox( QWidget *parent, QString title, QString message, bool htmlMsg, QString btn1, QString btn2, bool default1, bool default2 ) :
    MwcDialog(parent),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    ui->title->setText(title);

    // Setting text option
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textOption.setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    ui->text3->document()->setDefaultTextOption( textOption );

    // prepare for rendering
    QSize curSz = ui->text3->size();

    // text as a data should work for as.
    if (htmlMsg)
        ui->text3->setHtml( "<center>" + message + "</center>" );
    else
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

    if (btn1.isEmpty()) {
        ui->button1->hide();
        QLayoutItem * cntL = ui->central_HorizontalSpacer->layout()->takeAt(2);
        delete cntL;
        ui->central_HorizontalSpacer = nullptr;
    }
    else {
        ui->button1->setText(btn1);
        ui->button1->setDefault(default1);
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
        ui->button2->setDefault(default2);
        ui->button2->setFocus();
    }

    ui->button2->adjustSize();
    ui->button1->adjustSize();

    adjustSize();
}

MessageBox::~MessageBox()
{
    delete ui;
}

void MessageBox::on_button1_clicked()
{
    retCode = RETURN_CODE::BTN1;
    accept();
}

void MessageBox::on_button2_clicked()
{
    retCode = RETURN_CODE::BTN2;
    accept();
}

// One button, OK box
//static
void MessageBox::messageText( QWidget *parent, QString title, QString message ) {
    MessageBox * msgBox = new MessageBox(parent, title, message, false, "OK", "", true,false);
    msgBox->exec();
    delete msgBox;
}

void MessageBox::messageHTML( QWidget *parent, QString title, QString message ) {
    MessageBox * msgBox = new MessageBox(parent, title, message, true, "OK", "", true,false);
    msgBox->exec();
    delete msgBox;
}

// Two button box
//static
MessageBox::RETURN_CODE MessageBox::questionText( QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1, bool default2 ) {
    MessageBox * msgBox = new MessageBox(parent, title, message, false, btn1, btn2, default1, default2);
    msgBox->exec();
    RETURN_CODE  res = msgBox->getRetCode();
    delete msgBox;
    return res;
}
MessageBox::RETURN_CODE MessageBox::questionHTML( QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1, bool default2 ) {
    MessageBox * msgBox = new MessageBox(parent, title, message, true, btn1, btn2, default1, default2);
    msgBox->exec();
    RETURN_CODE  res = msgBox->getRetCode();
    delete msgBox;
    return res;
}



}


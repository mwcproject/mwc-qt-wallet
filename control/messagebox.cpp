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

namespace control {

MessageBox::MessageBox( QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1, bool default2 ) :
    MwcDialog(parent),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    ui->title->setText(title);
    ui->text->setText(message);

    if (btn1.isEmpty()) {
        ui->button1->hide();
        QLayoutItem * cntL = ui->central_HorizontalSpacer->layout()->takeAt(2);
        delete cntL;
        ui->central_HorizontalSpacer = nullptr;
    }
    else {
        ui->button1->setText(btn1);
        ui->button1->setDefault(default1);
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
    }

    adjustSize();
}

MessageBox::~MessageBox()
{
    delete ui;
}

void MessageBox::on_button1_clicked()
{
    retCode = BTN1;
    accept();
}

void MessageBox::on_button2_clicked()
{
    retCode = BTN2;
    accept();
}

// One button, OK box
//static
void MessageBox::message( QWidget *parent, QString title, QString message ) {
    MessageBox * msgBox = new MessageBox(parent, title, message, "OK", "", true,false);
    msgBox->exec();
    delete msgBox;
}

// Two button box
//static
MessageBox::RETURN_CODE MessageBox::question( QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1, bool default2 ) {
    MessageBox * msgBox = new MessageBox(parent, title, message, btn1, btn2, default1, default2);
    msgBox->exec();
    RETURN_CODE  res = msgBox->getRetCode();
    delete msgBox;
    return res;
}



}


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

#include "control/inputdialog.h"
#include "ui_inputdialog.h"


namespace control {

InputDialog::InputDialog(QWidget *parent, QString title, QString message, QString placeholder, QString text) :
        MwcDialog(parent),
        ui(new Ui::InputDialog)
{
    ui->setupUi(this);
    
    ui->title->setText(title);
    ui->commentText->setText(message);
    if (!placeholder.isEmpty())
        ui->edit->setPlaceholderText(placeholder);

    if (!text.isEmpty())
        ui->edit->setText(text);
}

InputDialog::~InputDialog() {
    delete ui;
}

QString InputDialog::getText(QWidget *parent, QString title, QString message, QString placeholder, QString text2init, bool * ok ) {
    InputDialog *initDlg = new InputDialog(parent, title, message, placeholder, text2init);
    bool okExit = (initDlg->exec() == QDialog::Accepted);
    QString text = initDlg->getText();
    delete initDlg;
    if (ok)
        *ok = okExit;
    return text;
}

void InputDialog::on_cancelBtn_clicked() {
    text = ui->edit->text().trimmed();
    reject();
}

void InputDialog::on_okBtn_clicked() {
    text = ui->edit->text().trimmed();
    accept();
}

}

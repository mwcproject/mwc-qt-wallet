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

#include "dialogs/x_shownotificationdlg.h"
#include "ui_x_shownotificationdlg.h"

namespace dlg {

ShowNotificationDlg::ShowNotificationDlg(notify::NotificationMessage msg, QWidget *parent) :
        MwcDialog(parent),
        ui(new Ui::ShowNotificationDlg) {
    ui->setupUi(this);

    ui->timeText->setText( msg.time.toString("ddd MMMM d yyyy HH:mm:ss") );
    ui->levelText->setText( msg.getLevelLongStr() );
    ui->messageText->setPlainText( msg.message );
}

ShowNotificationDlg::~ShowNotificationDlg() {
    delete ui;
}

void ShowNotificationDlg::on_pushButton_clicked()
{
    accept();
}

}


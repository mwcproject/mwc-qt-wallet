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

#include "showwalletstoppingmessagedlg.h"
#include "ui_showwalletstoppingmessagedlg.h"

namespace dlg {

ShowWalletStoppingMessageDlg::ShowWalletStoppingMessageDlg(QWidget *parent, int taskTimeout) :
    control::MwcDialog(parent),
    ui(new Ui::ShowWalletStoppingMessageDlg)
{
    ui->setupUi(this);

    QString waitingTime;
    if (taskTimeout < 120*1000 ) {
        waitingTime = QString::number(taskTimeout/1000) + " seconds";
    }
    else {
        waitingTime = QString::number(taskTimeout/1000/60) + " minutes";
    }

    ui->stoppingLabel->setText("mwc713 stopping might take up to <b>" + waitingTime + "</b> because it need to finish current running task.\n\n"
                               "Please wait until mwc713 will finish the task and exit.");
}

ShowWalletStoppingMessageDlg::~ShowWalletStoppingMessageDlg()
{
    delete ui;
}

}


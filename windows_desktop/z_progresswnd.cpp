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

#include "z_progresswnd.h"
#include "ui_z_progresswnd.h"
#include "../bridge/wnd/z_progresswnd_b.h"

namespace wnd {

ProgressWnd::ProgressWnd(QWidget *parent, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable ) :
    core::PanelBaseWnd(parent),
    ui(new Ui::ProgressWnd)
{
    ui->setupUi(this);
    progressWnd = new bridge::ProgressWnd(this);
    progressWnd->setCallerId(callerId);

    connect(progressWnd, &bridge::ProgressWnd::sgnSetHeader, this,  &ProgressWnd::onSgnSetHeader, Qt::QueuedConnection );
    connect(progressWnd, &bridge::ProgressWnd::sgnSetMsgPlus, this,  &ProgressWnd::onSgnSetMsgPlus, Qt::QueuedConnection );
    connect(progressWnd, &bridge::ProgressWnd::sgnInitProgress, this,  &ProgressWnd::onSgnInitProgress, Qt::QueuedConnection );
    connect(progressWnd, &bridge::ProgressWnd::sgnUpdateProgress, this,  &ProgressWnd::onSgnUpdateProgress, Qt::QueuedConnection );

    onSgnSetHeader(header);
    onSgnUpdateProgress(0,msgProgress);

    onSgnSetMsgPlus(msgPlus);

    if (!cancellable)
        ui->cancelButton->hide();
}

ProgressWnd::~ProgressWnd()
{
    delete ui;
}

void ProgressWnd::on_cancelButton_clicked()
{
    progressWnd->cancelProgress();
}

void ProgressWnd::onSgnSetHeader(QString header) {
    if (header.length()>0) {
        ui->header->setText(header);
        ui->header->show();
    }
    else {
        ui->header->hide();
    }

}
void ProgressWnd::onSgnSetMsgPlus(QString msgPlus) {
    if (msgPlus.length()>0) {
        ui->messagePlus->setText(msgPlus);
        ui->messagePlus->show();
    }
    else {
        ui->messagePlus->hide();
    }
}

void ProgressWnd::onSgnInitProgress(int min, int max) {
    ui->progressBar->setRange(min,max);
}

void ProgressWnd::onSgnUpdateProgress(int pos, QString msgProgress) {
    ui->progressBar->setValue(pos);

    if (msgProgress.length()>0) {
        ui->messageProgress->setText(msgProgress);
        ui->messageProgress->show();
    }
    else {
        ui->messageProgress->hide();
    }
}



}

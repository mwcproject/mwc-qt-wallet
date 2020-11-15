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

#include "s_swapbackupdlg.h"
#include "ui_s_swapbackupdlg.h"
#include "../bridge/config_b.h"
#include "../bridge/swap_b.h"
#include <QFileDialog>
#include "../core/WndManager.h"
#include "../control_desktop/messagebox.h"

namespace dlg {

SwapBackupDlg::SwapBackupDlg(QWidget *parent, QString _swapId, int _backupId) :
    control::MwcDialog(parent),
    ui(new Ui::SwapBackupDlg),
    swapId(_swapId),
    backupId(_backupId)
{
    ui->setupUi(this);

    ui->title->setText("Backup Trade " + swapId);
    ui->label1->setText("Please backup a data for the Swap Trade " + swapId + ".");

    config = new bridge::Config(this);
    swap = new bridge::Swap(this);

    connect(swap, &bridge::Swap::sgnBackupSwapTradeData, this, &SwapBackupDlg::sgnBackupSwapTradeData, Qt::QueuedConnection);

    QString path = config->getPathFor("SwapBackupDlg", true);
    if (!path.isEmpty())
        ui->backupDataPath->setText(path + QDir::separator() + "trade_" + swapId + ".trade" );

    updateButtons();
}

SwapBackupDlg::~SwapBackupDlg()
{
    delete ui;
}

void SwapBackupDlg::on_selectPath_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Trade Backup"),
                                                   config->getPathFor("SwapBackupDlg"),
                                                   tr("Trade Backup (*.trade)"));

    if (fileName.length()==0)
        return;
    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return;
    }

    if (!fileName.endsWith(".trade"))
        fileName += ".trade";

    config->updatePathFor("SwapBackupDlg", QFileInfo(fileName).absolutePath() );
    ui->backupDataPath->setText(fileName);

    updateButtons();

    on_backupButton_clicked();
}

void SwapBackupDlg::on_backupDataPath_textEdited(const QString &str)
{
    Q_UNUSED(str);
    updateButtons();
}

void SwapBackupDlg::updateButtons() {
    ui->skipButton->setEnabled( backupId>1 || !config->getSwapEnforceBackup() );
    ui->backupButton->setEnabled( !ui->backupDataPath->text().isEmpty() );
}


void SwapBackupDlg::on_skipButton_clicked()
{
    reject();
}

void SwapBackupDlg::on_backupButton_clicked()
{
    QString backupFilePath = ui->backupDataPath->text();
    if (backupFilePath.isEmpty()) {
        core::getWndManager()->messageTextDlg("Input", "Please specify the backup file name");
    }

    auto fileOk = util::validateMwc713Str(backupFilePath);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return;
    }

    swap->backupSwapTradeData(swapId, backupFilePath);

    ui->backupButton->setEnabled(false);
}

void SwapBackupDlg::sgnBackupSwapTradeData(QString _swapId, QString exportedFileName, QString errorMessage) {
    Q_UNUSED(exportedFileName)

    if (swapId != _swapId)
        return;

    ui->backupButton->setEnabled(true);

    if (errorMessage.isEmpty()){
        config->setSwapBackStatus(swapId, backupId);

        accept(); // We are done, exiting
        return;
    }

    control::MessageBox::messageText(this, "Unable to backup", "Unable to backup the swap trade " + swapId + ".\n\n" + errorMessage);
}


}


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
#include "../bridge/wnd/swap_b.h"
#include "../control_desktop/messagebox.h"
#include <QFileDialog>

namespace dlg {

SwapBackupDlg::SwapBackupDlg(QWidget *parent) :
    control::MwcDialog(parent),
    ui(new Ui::SwapBackupDlg)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    swap = new bridge::Swap(this);

    ui->swapBackupDir->setText( config->getSwapBackupDir() );
}

SwapBackupDlg::~SwapBackupDlg()
{
    delete ui;
}


void dlg::SwapBackupDlg::on_selectBackupDirBtn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "Select trade backup directory");
    if (dir.isEmpty())
        return;

    ui->swapBackupDir->setText(dir);
    config->setSwapBackupDir(dir);
}

void dlg::SwapBackupDlg::on_okBtn_clicked()
{
    // Let's test the path value
    QString backupPath = ui->swapBackupDir->text();
    if (backupPath.isEmpty()) {
        control::MessageBox::messageText(this, "Input", "Please specify the atomic swap trades backup directory.");
        ui->swapBackupDir->setFocus();
        return;
    }

    if ( !swap->verifyBackupDir(backupPath, false) ) {
        control::MessageBox::messageText(this, "Input", "Please specify valid atomic swap trades backup directory.");
        ui->swapBackupDir->setFocus();
        return;
    }

    config->setSwapBackupDir(backupPath);

    accept();
}

}

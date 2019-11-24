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

#include "g_finalizeupload_w.h"
#include "ui_g_finalizeupload.h"
#include "../state/g_Finalize.h"
#include <QFileDialog>
#include "../util/Json.h"
#include "../control/messagebox.h"

namespace wnd {

FinalizeUpload::FinalizeUpload(QWidget *parent, state::Finalize * _state) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::FinalizeUpload),
    state(_state)
{
    ui->setupUi(this);
}

FinalizeUpload::~FinalizeUpload()
{
    state->deleteFinalizeWnd(this);
    delete ui;
}

void FinalizeUpload::on_uploadFileBtn_clicked() {
    // Logic is implemented into This Window
    // It is really wrong, but also we don't want to have special state for that.
    QString fileName = QFileDialog::getOpenFileName(this, tr("Finalize transaction file"),
                                                    state->getFileGenerationPath(),
                                                    tr("MWC response transaction (*.tx.response)"));

    if (fileName.length() == 0) {
        return;
    }

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath(flInfo.path());

    util::FileTransactionInfo transInfo;

    if (!transInfo.parseTransaction(fileName)) {
        control::MessageBox::messageText(this, "Incorrect File",
                                     "MWC response transaction file " + fileName + " has a wrong format.");
        return;
    }

    state->fileTransactionUploaded(fileName, transInfo);
}



}



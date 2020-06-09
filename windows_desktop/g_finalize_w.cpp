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

#include "g_finalize_w.h"
#include "ui_g_finalize.h"
#include <QFileDialog>
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/g_finalize_b.h"

namespace wnd {

Finalize::Finalize(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::FinalizeUpload)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    finalize = new bridge::Finalize(this);


}

Finalize::~Finalize()
{
    delete ui;
}

void Finalize::on_uploadFileBtn_clicked() {

    util::TimeoutLockObject to("FinalizeUpload");

    if ( !finalize->isNodeHealthy() ) {
        control::MessageBox::messageText(this, "Unable to finalize", "Your MWC-Node, that wallet connected to, is not ready to finalize transactions.\n"
                                                                     "MWC-Node need to be connected to few peers and finish blocks synchronization process");
        return;
    }

    // Logic is implemented into This Window
    // It is really wrong, but also we don't want to have special state for that.
    QString fileName = QFileDialog::getOpenFileName(this, tr("Finalize transaction file"),
                                                    config->getPathFor("fileGen"),
                                                    tr("MWC response transaction (*.tx.response *.response);;All files (*.*)"));

    if (fileName.length() == 0) {
        return;
    }

    // Update path
    QFileInfo flInfo(fileName);
    config->updatePathFor("fileGen", flInfo.path());

    finalize->uploadFileTransaction(fileName);
}



}



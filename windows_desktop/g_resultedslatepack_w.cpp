// Copyright 2021 The MWC Developers
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

#include "g_resultedslatepack_w.h"
#include "ui_g_resultedslatepack_w.h"
#include "../control_desktop/QrCodeWidget.h"
#include "../bridge/statemachine_b.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"

namespace wnd {

ResultedSlatepack::ResultedSlatepack( QWidget *parent, QString slatepack, int backStateId, QString txExtension ) :
        core::NavWnd(parent),
        ui(new Ui::ResultedSlatepack) {
    ui->setupUi(this);
    this->backStateId   = backStateId;
    this->txExtension   = txExtension;
    this->slatepack     = slatepack;

    stateMachine = new bridge::StateMachine(this);
    config = new bridge::Config(this);
    util = new bridge::Util(this);

    ui->qr_code_window->setContent(slatepack);
    ui->slatepackContent->setText(slatepack);
}

ResultedSlatepack::~ResultedSlatepack() {
    delete ui;
}

void ResultedSlatepack::on_backButton_clicked() {
    stateMachine->setActionWindow( backStateId );
}

void ResultedSlatepack::on_saveQrImageBtn_clicked() {
    QString fileName = util->getSaveFileName("Save QR code image",
                                                    "ResultedSlatepack",
                                                    "QR code Image (*.png)", ".png");

    if (fileName.isEmpty())
        return;

    ui->qr_code_window->generateQrImage(fileName);
}

void ResultedSlatepack::on_saveSlatepackBtn_clicked() {
    QString fileName = util->getSaveFileName("Save Slatepack",
                                             "ResultedSlatepack",
                                             "Slatepack tramsaction (*"+txExtension+")", txExtension);

    if (fileName.isEmpty())
        return;

    // Write some text into the file
    util->writeTextFile(fileName, {slatepack} );
}

void ResultedSlatepack::on_backButton_2_clicked()
{
    // It is duplicate UI button
    on_backButton_clicked();
}

}

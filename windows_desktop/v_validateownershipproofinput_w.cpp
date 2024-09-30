// Copyright 2024 The MWC Developers
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

#include "v_validateownershipproofinput_w.h"
#include "ui_v_validateownershipproofinput_w.h"
#include "../bridge/wnd/v_viewoutputs_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../util_desktop/timeoutlock.h"
#include "../util/filedialog.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

ValidateOwnershipProofInput::ValidateOwnershipProofInput(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::ValidateOwnershipProofInput) {
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    viewAcounts = new bridge::ViewOutputs(this);
    util = new bridge::Util(this);

    updateButtons();
}

ValidateOwnershipProofInput::~ValidateOwnershipProofInput() {
    delete ui;
}

void ValidateOwnershipProofInput::updateButtons() {
    ui->validate->setEnabled(!ui->proof_text->toPlainText().isEmpty());
}


void ValidateOwnershipProofInput::on_proof_text_textChanged()
{
    updateButtons();
}


void ValidateOwnershipProofInput::on_back_clicked()
{
    viewAcounts->back();
}


void ValidateOwnershipProofInput::on_load_from_file_clicked()
{
        util::TimeoutLockObject to("ValidateOwnershipProofInput");

        // Logic is implemented into This Window
        // It is really wrong, but also we don't want to have special state for that.
        QString fileName = util->getOpenFileName( "Load Ownership Proof",
                                                  "ValidateOwnership",
                                                  "Ownership Proof (*.proof);;All files (*.*)");
        if (fileName.isEmpty())
            return;

        QStringList lns = util->readTextFile(fileName);
        QString msg;
        for (const auto & ln : lns) {
            if (!ln.isEmpty())
                msg += ln;
        }

        if (!msg.isEmpty())
            ui->proof_text->setText(msg);

        updateButtons();
}


void ValidateOwnershipProofInput::on_validate_clicked()
{
    QString proof = ui->proof_text->toPlainText();
    if (!proof.isEmpty())
        viewAcounts->validate_proof(proof);
}

}

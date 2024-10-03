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

#include "v_generateownershipproofresult_w.h"
#include "ui_v_generateownershipproofresult_w.h"
#include "../bridge/wnd/v_viewoutputs_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../util_desktop/timeoutlock.h"
#include "../util/filedialog.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

GenerateOwnershipProofResult::GenerateOwnershipProofResult(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::GenerateOwnershipProofResult) {

    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    viewAcounts = new bridge::ViewOutputs(this);
    util = new bridge::Util(this);

    ui->save->setEnabled(false);

    QObject::connect(wallet, &bridge::Wallet::sgnGenerateOwnershipProof,
                     this, &GenerateOwnershipProofResult::onGenerateOwnershipProof, Qt::QueuedConnection);
}

GenerateOwnershipProofResult::~GenerateOwnershipProofResult() {
    delete ui;
}

void GenerateOwnershipProofResult::onGenerateOwnershipProof(QString proof, QString error) {
    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Error", "Unable to generate ownership proof.\n\nError:" + error);
        viewAcounts->back();
    }

    if (!proof.isEmpty()) {
        ui->proof_text->setText(proof);
        ui->save->setEnabled(true);
    }
}


void GenerateOwnershipProofResult::on_back_clicked()
{
    viewAcounts->back();
}


void GenerateOwnershipProofResult::on_save_clicked()
{
    QString  text = ui->proof_text->toPlainText();
    if (!text.isEmpty()) {

        util::TimeoutLockObject to("GenerateOwnershipProofResult");

        // Logic is implemented into This Window
        // It is really wrong, but also we don't want to have special state for that.
        QString fileName = util->getSaveFileName( "Save Ownership Proof",
                                                        "GenOwnership",
                                                        "Ownership Proof (*.proof)", ".proof");
        if (fileName.isEmpty())
            return;

        QStringList lns;
        lns.append(text);
        if(!util->writeTextFile(fileName, lns)) {
            control::MessageBox::messageText(this, "Error", "Unable to save proof into the file\n" + fileName);
        }
    }
}

}

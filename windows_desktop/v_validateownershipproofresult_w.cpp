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

#include "v_validateownershipproofresult_w.h"
#include "ui_v_validateownershipproofresult_w.h"
#include "../bridge/wnd/v_viewoutputs_b.h"
#include "../bridge/wallet_b.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/widgetutils.h"

namespace wnd {

ValidateOwnershipProofResult::ValidateOwnershipProofResult(QWidget *parent, const QString & network, const QString & message, const QString & viewingKey,
    const QString & torAddress, const QString & mqsAddress) :
        core::NavWnd(parent),
        ui(new Ui::ValidateOwnershipProofResult)
{
    ui->setupUi(this);

    viewAcounts = new bridge::ViewOutputs(this);

    utils::defineDefaultButtonSlot(this, SLOT(on_back_clicked()) );

    ui->network->setText(network);
    ui->message->setText(message);
    ui->viewing_key->setText(viewingKey);
    ui->tor_address->setText(torAddress);
    ui->mqs_address->setText(mqsAddress);
}

ValidateOwnershipProofResult::~ValidateOwnershipProofResult() {
    delete ui;
}

void ValidateOwnershipProofResult::on_back_clicked() {
    viewAcounts->back();
}

}

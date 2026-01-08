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


#include "a_seedlength.h"
#include "ui_a_seedlength.h"
#include "../bridge/wnd/a_initaccount_b.h"
#include "../util_desktop/widgetutils.h"

namespace wnd {

SeedLength::SeedLength(QWidget *parent) :
        control::MwcWidget(parent),
        ui(new Ui::SeedLength) {
    ui->setupUi(this);

    initAccount = new bridge::InitAccount(this);

    utils::defineDefaultButtonSlot(this, SLOT(on_continueButton_clicked()) );
}

SeedLength::~SeedLength() {
    delete ui;
}

void SeedLength::on_continueButton_clicked()
{
    int length = 24;
    if (ui->radioLength12->isChecked())
        length = 12;
    else if (ui->radioLength15->isChecked())
        length = 15;
    else if (ui->radioLength18->isChecked())
        length = 18;
    else if (ui->radioLength21->isChecked())
        length = 21;
    else
        length = 24;

    initAccount->submitSeedLength(length);
}

}

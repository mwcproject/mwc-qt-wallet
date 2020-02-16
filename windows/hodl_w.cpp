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

#include "hodl_w.h"
#include "ui_hodl.h"
#include "../state/hodl.h"
#include "../control/messagebox.h"
#include <QAbstractItemView>
#include "../state/timeoutlock.h"
#include <QProcess>
#include "../util/execute.h"

namespace wnd {

Hodl::Hodl(QWidget *parent, state::Hodl * _state) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::Hodl),
    state(_state)
{
    ui->setupUi(this);

}

Hodl::~Hodl()
{
    delete ui;
}

void Hodl::on_pushButton_clicked()
{
    util::openUrlInBrowser("https://www.mwc.mw/contact");
}


}



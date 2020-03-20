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

#include "a_waitingwnd.h"
#include "ui_a_waitingwnd.h"
#include <QMovie>
#include <QLabel>

namespace wnd {

WaitingWnd::WaitingWnd(QWidget *parent, WaitingWndState * _state, QString title, QString progressMessage) :
    QWidget(parent),
    ui(new Ui::WaitingWnd),
    state(_state)
{
    ui->setupUi(this);

    updateTitle(title);
    updateProgressMessage(progressMessage);

    ui->progress->initLoader(true);
}

WaitingWnd::~WaitingWnd()
{
    state->deleteWaitingWnd(this);
    delete ui;
}

void WaitingWnd::updateTitle(QString title) {
    ui->titleLabel->setText("<html><body><p><span style=\" font-size:18pt; font-weight:600;\">" + title + "</span></p></body></html>");
}

void WaitingWnd::updateProgressMessage( QString message ) {
    ui->progressMsg->setText(message);
}

}

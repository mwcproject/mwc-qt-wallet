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

#ifndef NEWSEEDTESTW_H
#define NEWSEEDTESTW_H

#include "../core/PanelBaseWnd.h"

namespace Ui {
class NewSeedTest;
}

class QCompleter;

namespace state {
    class InitAccount;
}

namespace wnd {

class NewSeedTest : public core::PanelBaseWnd
{
    Q_OBJECT

public:
    explicit NewSeedTest(QWidget *parent, state::InitAccount *state, int wordNumber);
    ~NewSeedTest();

private slots:
    void on_submitButton_clicked();

private:
    Ui::NewSeedTest *ui;
    state::InitAccount *state;
    QCompleter *completer = nullptr;
};

}


#endif // NEWSEEDTESTW_H

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

#ifndef NEWWALLETW_H
#define NEWWALLETW_H

#include <QWidget>

namespace Ui {
class NewWallet;
}

namespace state {
    class InitAccount;
}

namespace wnd {

class NewWallet : public QWidget
{
    Q_OBJECT

public:
    explicit NewWallet(QWidget *parent, state::InitAccount * state);
    virtual ~NewWallet() override;

private slots:
    void on_submitButton_clicked();

    void on_radioHaveSeed_clicked();
    void on_radioCreateNew_clicked();

private:
    void updateControls();
private:
    Ui::NewWallet *ui;
    state::InitAccount * state = nullptr;
};

}

#endif // NEWWALLET_H

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

#ifndef INPUTPASSWORDW_H
#define INPUTPASSWORDW_H

#include <QWidget>

namespace Ui {
class InputPassword;
}

namespace state {
    class InputPassword;
    class WalletConfig;
}

namespace wnd {

class InputPassword : public QWidget
{
    Q_OBJECT

public:
    explicit InputPassword(QWidget *parent, state::InputPassword * state, state::WalletConfig * configState, bool lockMode);
    ~InputPassword();

    void startWaiting();
    void stopWaiting();

    void reportWrongPassword();

    void updateMwcMqState(bool online);
    void updateKeybaseState(bool online);
    void updateHttpState(bool online);

private slots:
    void on_submitButton_clicked();

    void on_instancesButton_clicked();

    void onUpdateSyncProgress(double progressPercent);
private:
    Ui::InputPassword *ui;
    state::InputPassword * state;
    state::WalletConfig * configState;
};

}

#endif // INPUTPASSWORDW_H

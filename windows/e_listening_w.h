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

#ifndef LISTENINGW_H
#define LISTENINGW_H

#include "../core/navwnd.h"

namespace Ui {
class Listening;
}

namespace state {
    class Listening;
}

namespace wnd {

class Listening : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Listening(QWidget *parent, state::Listening * state, bool mwcMqStatus, bool keybaseStatus,
                       QString mwcMqAddress, int mwcMqAddrIdx);
    ~Listening();

    void updateMwcMqState(bool online);
    void updateKeybaseState(bool online);
    void updateMwcMqAddress(QString address, int addrIdx);

    void showMessage(QString title, QString message);

private slots:
    void on_mwcMqTriggerButton_clicked();

    void on_mwcMqNextAddress_clicked();

    void on_mwcMqToIndex_clicked();

    void on_keybaseTriggerButton_clicked();

private:


private:
    Ui::Listening *ui;
    state::Listening * state;
};

}

#endif // LISTENINGW_H

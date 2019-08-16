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

#ifndef EVENTSW_H
#define EVENTSW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Events;
}

namespace state {
    class Events;
}

namespace wnd {

class Events : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Events(QWidget *parent, state::Events * state);
    ~Events();

    void updateShowMessages();

private slots:
    void on_notificationList_cellActivated(int row, int column);

private:
    void initTableHeaders();
    void saveTableHeaders();

private:
    Ui::Events *ui;
    state::Events * state;

    QVector<wallet::WalletNotificationMessages> messages; // messaged that currently on diplay
};

}

#endif // EVENTS_H

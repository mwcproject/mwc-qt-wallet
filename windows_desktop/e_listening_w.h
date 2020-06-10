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

#include "../core_desktop/navwnd.h"

namespace Ui {
class Listening;
}

namespace bridge {
class Wallet;
class Config;
}

namespace wnd {

class Listening : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Listening(QWidget *parent);
    ~Listening();

private slots:
    void onSgnUpdateListenerStatus(bool mwcOnline, bool keybaseOnline, bool tor);
    void onSgnHttpListeningStatus(bool listening, QString additionalInfo);
    void onSgnMwcAddressWithIndex(QString mwcAddress, int idx);

    void on_mwcMqTriggerButton_clicked();
    void on_mwcMqNextAddress_clicked();
    void on_mwcMqToIndex_clicked();
    void on_keybaseTriggerButton_clicked();
    void on_httpConfigButton_clicked();
    void on_torTriggerButton_clicked();

private:
    void updateStatuses();
    void updateMwcMqAddress(QString address, int addrIdx);
private:
    Ui::Listening *ui;
    bridge::Wallet * wallet = nullptr;
    bridge::Config * config = nullptr;
};

}

#endif // LISTENINGW_H

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

#ifndef SEND3_OFFLINESETTINGS_H
#define SEND3_OFFLINESETTINGS_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class SendOffline;
}

namespace state {
class Send;
}


namespace wnd {

class SendOffline : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendOffline(QWidget *parent, const wallet::AccountInfo & selectedAccount, int64_t amount, state::Send * state);
    virtual ~SendOffline() override;

    void showSendMwcOfflineResult( bool success, QString message );
private slots:
    void on_sendButton_clicked();

    void on_settingsBtn_clicked();

private:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    Ui::SendOffline * ui;
    state::Send * state;

    wallet::AccountInfo selectedAccount;
    int64_t amount;
};

}

#endif // SEND3_OFFLINESETTINGS_H

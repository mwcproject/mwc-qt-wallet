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

#ifndef SEND2_ONLINE_H
#define SEND2_ONLINE_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class SendOnline;
}

namespace bridge {
class Util;
class Config;
class Send;
}

namespace wnd {

class SendOnline : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendOnline(QWidget *parent, QString account, int64_t amount );
    virtual ~SendOnline() override;

private slots:
    void on_contactsButton_clicked();
    void on_sendButton_clicked();
    void on_sendEdit_textEdited(const QString &arg1);
    void on_settingsBtn_clicked();
    void on_sendEdit_textChanged(const QString &arg1);

    void onSgnShowSendResult( bool success, QString message );
private:
    Ui::SendOnline *ui;
    bridge::Util * util = nullptr;
    bridge::Config * config = nullptr;
    bridge::Send * send = nullptr;

    QString account;
    int64_t amount;
};


}


#endif // SEND2_ONLINE_H

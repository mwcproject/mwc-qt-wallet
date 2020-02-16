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

#ifndef SEND1_ONLINEOFFLINE_W_H
#define SEND1_ONLINEOFFLINE_W_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace Ui {
class SendStarting;
}

namespace state {
    class Send;
}

namespace wnd {

class SendStarting : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendStarting(QWidget *parent, state::Send * state);
    ~SendStarting();

    void updateAccountBalance( QVector<wallet::AccountInfo> accountInfo, const QString & selectedAccount );

private slots:

    void onChecked(int id);

    void on_nextButton_clicked();
    void on_allAmountButton_clicked();

private:
    Ui::SendStarting *ui;
    state::Send * state;
    QVector<wallet::AccountInfo> accountInfo;
};

QString generateAmountErrorMsg( int64_t mwcAmount, const wallet::AccountInfo & acc, const core::SendCoinsParams & sendParams );


}

#endif // SEND1_ONLINEOFFLINE_W_H

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

#ifndef E_RECIEVE_W_H
#define E_RECIEVE_W_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Recieve;
}

namespace state {
class Recieve;
}

namespace wnd {

class Recieve : public core::NavWnd {
Q_OBJECT

public:
    explicit Recieve(QWidget *parent, state::Recieve * state, bool mwcMqStatus, bool keybaseStatus,
                     QString mwcMqAddress);

    virtual ~Recieve() override ;

    void updateMwcMqAddress(QString address);
    void updateMwcMqState(bool online);
    void updateKeybaseState(bool online);

    void onTransactionActionIsFinished( bool success, QString message );
    void stopWaiting();

private slots:
    void on_pushButton_clicked();

    void on_accountComboBox_activated(int index);

private:
    void updateAccountList();

private:
    Ui::Recieve *ui;
    state::Recieve * state;
    QVector<wallet::AccountInfo> accountInfo;
    QString mwcAddress;
};

}

#endif // E_RECIEVE_W_H

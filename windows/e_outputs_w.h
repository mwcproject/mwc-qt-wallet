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

#ifndef OUTPUTSW_H
#define OUTPUTSW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Outputs;
}

namespace state {
    class Outputs;
}

namespace wnd {

class Outputs : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Outputs(QWidget *parent, state::Outputs * state);
    ~Outputs();

    void setOutputCount(QString account, int count);
    void setOutputsData(QString account, int64_t height, const QVector<wallet::WalletOutput> & outp );

    // return selected account
    QString updateWalletBalance();

private slots:
    void on_accountComboBox_activated(int index);

    void on_prevBtn_clicked();

    void on_nextBtn_clicked();

    void on_refreshButton_clicked();

private:
    void initTableHeaders();
    void saveTableHeaders();

    void requestOutputs(QString account);

    QString currentSelectedAccount();

    // return enable state for the buttons
    QPair<bool,bool> updatePages( int currentPos, int total, int pageSize );

    int calcPageSize() const;

    virtual void timerEvent(QTimerEvent *event);

private:
    Ui::Outputs *ui;
    state::Outputs * state;
    QVector<wallet::AccountInfo> accountInfo;
    QVector<wallet::WalletOutput> outputs;

    int currentPagePosition = INT_MAX; // position at the paging...
    int totalOutputs = 0;

    QPair<bool,bool> buttonState = QPair<bool,bool>(false, false);
};

}

#endif // OUTPUTS_H

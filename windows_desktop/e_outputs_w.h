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

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"
#include "../control_desktop/richbutton.h"

class QLabel;

namespace Ui {
class Outputs;
}

namespace bridge {
class Config;
class Wallet;
class Outputs;
}

namespace control {
class RichItem;
}

namespace wnd {

struct OutputData {
    wallet::WalletOutput output;
    control::RichItem * ritem = nullptr;
    QWidget * markWnd = nullptr;
    control::RichButton * lockBtn = nullptr;
    QLabel * lockLabel = nullptr;
    QLabel * comment = nullptr;

    void setBtns(control::RichItem * ritem, QWidget * markWnd, control::RichButton * lockBtn, QLabel * lockLabel, QLabel * comment);
};

class Outputs : public core::NavWnd,  control::RichButtonPressCallback
{
    Q_OBJECT

public:
    explicit Outputs(QWidget *parent);
    ~Outputs();

private slots:
    void on_accountComboBox_activated(int index);
    void on_refreshButton_clicked();
    void on_showUnspent_clicked();

    void onSgnWalletBalanceUpdated();
    void onSgnOutputs( QString account, bool showSpent, QString height, QVector<QString> outputs);

    void onSgnNewNotificationMessage(int level, QString message);

    void onItemActivated(QString itemId);

protected:
    virtual void richButtonPressed(control::RichButton * button, QString coockie) override;

private:
    virtual void panelWndStarted() override;

    bool calcMarkFlag(const wallet::WalletOutput & out);

    bool updateOutputState(int idx, bool lock);

    void requestOutputs(QString account, bool resetScrollPos);

    QString currentSelectedAccount();

    // return enable state for the buttons
    void updateShownData(bool resetScrollData);

    // return selected account
    QString updateAccountsData();

    // return "N/A, YES, "NO"
    QString calcLockedState(const wallet::WalletOutput & output);

    // return true if user fine with lock changes
    bool showLockMessage();
private:
    Ui::Outputs *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Outputs * outputs = nullptr; // needed as output windows active flag

    QVector<OutputData> allData; // all outputs

    bool canLockOutputs = false;

    static bool lockMessageWasShown;
};

}

#endif // OUTPUTS_H

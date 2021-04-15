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

#ifndef MWCTOOLBAR_H
#define MWCTOOLBAR_H

#include <QWidget>
#include "../state/state.h"

namespace Ui {
class MwcToolbar;
}

namespace bridge {
    class Config;
    class Wallet;
    class StateMachine;
    class CoreWindow;
}


namespace core {

class MainWindow;

class MwcToolbar : public QWidget
{
   Q_OBJECT

public:
    explicit MwcToolbar(QWidget *parent);
    ~MwcToolbar() override;

protected:
    virtual void paintEvent(QPaintEvent *) override;
    virtual void timerEvent(QTimerEvent *event) override;

private slots:

    void onWalletBalanceUpdated();
    void onLoginResult(bool ok);
    void onLogout();

    // state: state::STATE
    void onUpdateButtonsState( int state );

    // UI
    void on_sendToolButton_clicked();
    void on_receiveToolButton_clicked();
    void on_transactionToolButton_clicked();
    void on_swapToolButton_clicked();
    void on_finalizeToolButton_clicked();

    void on_swapMarketplaceToolButton_clicked();

private:
    Ui::MwcToolbar *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::StateMachine * stateMachine = nullptr;
    bridge::CoreWindow * coreWindow = nullptr;
};

}

#endif // MWCTOOLBAR_H

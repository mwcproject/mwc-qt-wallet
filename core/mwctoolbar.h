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

namespace state {
class StateMachine;
}

namespace core {

class MainWindow;

class MwcToolbar : public QWidget
{
   Q_OBJECT

public:
    explicit MwcToolbar(QWidget *parent);
    ~MwcToolbar() override;

    void setAppEnvironment(state::StateMachine * stateMachine, wallet::Wallet * wallet );

    void updateButtonsState( state::STATE state );

protected:
    virtual void paintEvent(QPaintEvent *) override;

private slots:
    void on_airdropToolButton_clicked();

    void on_sendToolButton_clicked();

    void on_receiveToolButton_clicked();

    void on_transactionToolButton_clicked();

    void on_hodlToolButton_clicked();

    void onWalletBalanceUpdated();

    void on_finalizeToolButton_clicked();

private:
    Ui::MwcToolbar *ui;
    wallet::Wallet      * wallet = nullptr;
    state::StateMachine * stateMachine = nullptr;
};

}

#endif // MWCTOOLBAR_H

// Copyright 2020 The MWC Developers
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

#ifndef S_NEWSWAP_W_H
#define S_NEWSWAP_W_H

#include "../core_desktop/navwnd.h"

class QLineEdit;

namespace Ui {
class NewSwap;
}

namespace bridge {
class Swap;
}

namespace wnd {

class NewSwap : public QWidget {
Q_OBJECT
public:
    explicit NewSwap(QWidget *parent);
    ~NewSwap();

private:
    bool getValue( QLineEdit * edit, double min, double max, double & result, QString message );
    bool getValue( QLineEdit * edit, int min, int max, int & result, QString message );

    void updateRate();
    void updateSecondary();

private slots:
    void on_calcMwcConfBtn_clicked();
    void on_backBtn_clicked();
    void on_createBtn_clicked();

    // Response from createNewSwapTrade, SwapId on OK,  errMsg on failure
    void sgnCreateNewTradeResult( QString swapId, QString errMsg );

    void on_mwcAmountEdit_textEdited(const QString &arg1);

    void on_secondaryAmountEdit_textEdited(const QString &arg1);

    void on_exchangeRateEdit_textEdited(const QString &arg1);

private:
    Ui::NewSwap *ui;
    bridge::Swap * swap = nullptr;
};

}

#endif // S_NEWSWAP_W_H

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

#ifndef AIRDROPFORBTC_H
#define AIRDROPFORBTC_H

#include "../core_desktop/PanelBaseWnd.h"

namespace Ui {
class AirdropForBTC;
}

namespace bridge {
class Airdrop;
}

namespace wnd {

class AirdropForBTC : public core::PanelBaseWnd
{
    Q_OBJECT

public:
    explicit AirdropForBTC(QWidget *parent, QString btcAddress, QString challenge, QString identifier );
    ~AirdropForBTC();


private slots:
    void on_claimButton_clicked();
    void on_backButton_clicked();

    void onSgnReportMessage( QString title, QString message );
private:
    Ui::AirdropForBTC *ui;
    bridge::Airdrop * airdrop = nullptr;

    QString btcAddress;
    QString challenge;
    QString identifier;
};

}

#endif // AIRDROPFORBTC_H

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

#ifndef AIDROPW_H
#define AIDROPW_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class Airdrop;
}

namespace bridge {
class Airdrop;
class Config;
}

namespace wnd {

class Airdrop : public core::NavWnd
{
    Q_OBJECT
public:
    explicit Airdrop(QWidget *parent);
    ~Airdrop();

private slots:
    void on_nextButton_clicked();
    void on_refreshClaimsButton_clicked();
    void on_btcAddressEdit_textChanged(const QString &arg1);
    void on_claimAirdropBtn_clicked();

    void onSgnUpdateAirDropStatus( bool waiting, bool status, QString message );
    void onSgnUpdateClaimStatus( int idx, QString requestBtcAddress,
                                 QString status, QString message, QString mwc, int errCode);
    void onSgnReportMessage( QString title, QString message );
private:
    // initiate update status from the state
    void updateClaimStatus();

    void initTableHeaders();
    void saveTableHeaders();

    void showProgress(const QString & message);
    void hideProgress();
private:
    Ui::Airdrop *ui;
    bridge::Airdrop * airdrop = nullptr;
    bridge::Config * config = nullptr;
};

}

#endif // AIDROPW_H

// Copyright 2021 The MWC Developers
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

#ifndef S_MRKTSWLIST_W_H
#define S_MRKTSWLIST_W_H

#include "../core_desktop/navwnd.h"
#include "../control_desktop/richbutton.h"
#include "../state/s_mktswap.h"

namespace Ui {
class MrktSwList;
}

namespace bridge {
class Config;
class SwapMarketplace;
class Wallet;
class Util;
}

namespace wnd {

const int BTN_MKT_OFFERS = 0;
const int BTN_MY_OFFERS = 1;
const int BTN_FEES = 2;

const QVector<QPair<QString, double>> MKT_FEES{
        QPair<QString, double>("High", 0.005),
        QPair<QString, double>("Normal", 0.001),
        QPair<QString, double>("Low", 0.0001)
};
QString feeLevelValToStr(double fee);

class MrktSwList : public core::NavWnd, control::RichButtonPressCallback {
Q_OBJECT

public:
    explicit MrktSwList(QWidget *parent, bool selectMyOffers);

    ~MrktSwList();

protected:
    virtual void richButtonPressed(control::RichButton * button, QString coockie) override;

private:
    void updateModeButtons(int btnId);
    void updateMktFilter();
    void updateTradeListData();
    void pushIntegritySettings();
private slots:
    void on_marketOffers_clicked();
    void on_myOffers_clicked();
    void on_feesBtn_clicked();
    void on_feeLevel_currentIndexChanged(int index);
    void on_depositAccountName_currentIndexChanged(int index);
    void on_mwcReservedFee_textEdited(const QString &arg1);
    void on_withdrawFeesBtn_clicked();
    void on_feeLevelHelpBtn_clicked();
    void on_depositAccountHelpBtn_clicked();
    void on_reservedAmountHelpBtn_clicked();
    void on_activeFeesHelpBtn_clicked();
    void on_withdrawHelpBtn_clicked();
    void on_newOfferButton_clicked();
    void on_refreshButton_clicked();

    void on_offersListenSettingsBtn_clicked();

    void sgnRequestIntegrityFees(QString error, int64_t balance, QVector<QString> IntegrityFeesJsonStr);
    void sgnUpdateNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, double totalDifficulty, int connections );
    void sgnWithdrawIntegrityFees(QString error, double mwc, QString account);

    void sgnMarketPlaceOffersChanged();
    void sgnMyOffersChanged();
    void sgnMessagingStatusChanged();

    void onItemActivated(QString id);

private:
    Ui::MrktSwList *ui;
    bridge::Config * config = nullptr;
    bridge::SwapMarketplace * swapMarketplace = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Util * util = nullptr;

    int selectedTab = 0;
    int lastNodeHeight = 0;
};

}

#endif // S_MRKTSWLIST_W_H

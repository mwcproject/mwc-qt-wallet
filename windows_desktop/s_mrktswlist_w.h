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
class Swap;
class Wallet;
class Util;
}

namespace wnd {

const int BTN_MKT_OFFERS = 0;
const int BTN_MY_OFFERS = 1;
const int BTN_FEES = 2;

struct FeeInfo {
    QString shortName;
    QString longName;
    double  fee = 0.0;

    FeeInfo(QString _shortName, QString _longName, double _fee) :
            shortName(_shortName), longName(_longName), fee(_fee) {}

    FeeInfo() = default;
    FeeInfo(const FeeInfo & other) = default;
    FeeInfo & operator=(const FeeInfo & other) = default;
};

const QVector<FeeInfo> MKT_FEES{
        FeeInfo("High","High: 50 bps or 0.5%", 0.005),
        FeeInfo( "Normal", "Normal: \u2A7E 10 bps or 0.1%", 0.001),
        FeeInfo( "Low", "Low: \u2A7E 1 bps or 0.01%", 0.0001)
};
QString feeLevelValToStr(double fee);

class MrktSwList : public core::NavWnd, control::RichButtonPressCallback {
Q_OBJECT

public:
    explicit MrktSwList(QWidget *parent, bool selectMyOffers, bool selectFee);

    ~MrktSwList();

protected:
    virtual void richButtonPressed(control::RichButton * button, QString coockie) override;

private:
    void updateModeButtons(int btnId);
    void updateMktFilter();
    void updateTradeListData(bool resetScrollValue);
    void pushIntegritySettings();
    void updateMwcReservedFeeValidity();
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

    void sgnRequestIntegrityFees(QString error, int64_t balance, QVector<QString> IntegrityFeesJsonStr);
    void sgnWithdrawIntegrityFees(QString error, double mwc, QString account);

    void sgnMarketPlaceOffersChanged();
    void sgnMyOffersChanged();
    void sgnMessagingStatusChanged();

    void onBuyItemActivated(QString id);
    void onSellItemActivated(QString id);

    void updateList();
    void on_buySellCombo_currentIndexChanged(int index);

    void on_secCurrencyCombo_currentIndexChanged(int index);

    void on_filterFeeLevel_currentIndexChanged(int index);

    void on_viewIntegrityTransactionsBtn_clicked();

private:
    Ui::MrktSwList *ui;
    bridge::Config * config = nullptr;
    bridge::SwapMarketplace * swapMarketplace = nullptr;
    bridge::Swap * swap = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Util * util = nullptr;

    int selectedTab = 0;
    bool controlsReady = false;
};

}

#endif // S_MRKTSWLIST_W_H

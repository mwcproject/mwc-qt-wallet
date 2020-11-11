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

#ifndef S_SWAPLIST_W_H
#define S_SWAPLIST_W_H

#include "../core_desktop/navwnd.h"
#include "../control_desktop/richbutton.h"

class QLabel;
class QWidget;

namespace Ui {
class SwapList;
}

namespace bridge {
class Swap;
class Config;
class Util;
}

namespace control {
class RichItem;
}

namespace wnd {

struct SwapTradeInfo {
    bool    isSeller;
    QString mwcAmount;
    QString secondaryAmount;
    QString secondaryCurrency;
    QString tradeId;
    QString stateCmd;
    QString status; // action or state
    int64_t initiatedTime; // timestamp in seconds
    int64_t expirationTime; // timestamp in seconds
    QString secondary_address;

    QLabel * initTimeLable = nullptr;
    QLabel * expirationLable = nullptr;
    QLabel * statusLable = nullptr;
    QWidget * cancelBtn = nullptr;
    QWidget * deleteBtn = nullptr;
    QWidget * backupBtn = nullptr;
    QWidget * acceptBtn = nullptr;

    SwapTradeInfo() = default;
    SwapTradeInfo(const SwapTradeInfo & obj) = default;
    SwapTradeInfo & operator = (const SwapTradeInfo & obj) = default;

    SwapTradeInfo( bool _isSeller,
                   const QString & _mwcAmount,
                   const QString & _secondaryAmount,
                   const QString & _secondaryCurrency,
                   const QString & _tradeId,
                   const QString & _stateCmd,
                   const QString & _status,
                   int64_t         _initiatedTime,
                   int64_t         _expirationTime,
                   const QString & _secondary_address) :
            isSeller(_isSeller), mwcAmount(_mwcAmount), secondaryAmount(_secondaryAmount), secondaryCurrency(_secondaryCurrency),
            tradeId(_tradeId), stateCmd(_stateCmd), status(_status), initiatedTime(_initiatedTime), expirationTime(_expirationTime),
            secondary_address(_secondary_address) {}

    // Reset all UI related data
    void resetUI() {
        initTimeLable = expirationLable = statusLable = nullptr;
        cancelBtn = deleteBtn = backupBtn = acceptBtn = nullptr;
    }

    // Update current state and UI
    void updateData(QString stateCmd, QString status, int64_t expirationTime, bridge::Util * util, bridge::Config * config, bridge::Swap * swap);

    void applyState2Ui(bridge::Util * util, bridge::Config * config, bridge::Swap * swap);
};

class SwapList : public core::NavWnd, control::RichButtonPressCallback {
Q_OBJECT
public:
    explicit SwapList(QWidget *parent);
    ~SwapList();

private:
    void requestSwapList();
    void selectSwapTab(int selection);
    void updateTradeListData();

protected:
    virtual void richButtonPressed(control::RichButton * button, QString coockie);

private slots:
    void sgnSwapTradesResult( QString cookie, QVector<QString> trades, QString error );
    void sgnDeleteSwapTrade(QString swapId, QString error);
    void sgnSwapTradeStatusUpdated(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                                   int64_t expirationTime,
                                   QVector<QString> executionPlan,
                                   QVector<QString> tradeJournal);
    void sgnNewSwapTrade(QString currency, QString swapId);
    void sgnCancelTrade(QString swapId, QString error);

    void sgnBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage);
    void sgnRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);

    void onItemActivated(QString id);

    void on_outgoingSwaps_clicked();
    void on_incomingSwaps_clicked();
    void on_completedSwaps_clicked();
    void on_newTradeButton_clicked();
    void on_refreshButton_clicked();


    void on_restoreTradesTab_clicked();

    void on_restoreTradeBtn_clicked();

    void on_checkEnforceBackup_clicked();

private:
    Ui::SwapList *ui;
    bridge::Swap * swap = nullptr;
    bridge::Config * config = nullptr;
    bridge::Util * util = nullptr;

    QVector<SwapTradeInfo> swapList;
    int swapTabSelection = 0; // 0 - 2 as shown in UI: incoming, outgoing, complete
};

}

#endif // S_SWAPLIST_W_H

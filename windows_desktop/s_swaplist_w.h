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

namespace Ui {
class SwapList;
}

namespace bridge {
class Swap;
class Config;
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
    QString state;
    QString initiatedTimeInterval; // String format as Brent defines it. It has special format
    QString expirationTimeInterval;
    QString secondary_address;

    SwapTradeInfo() = default;
    SwapTradeInfo(const SwapTradeInfo & obj) = default;
    SwapTradeInfo & operator = (const SwapTradeInfo & obj) = default;

    SwapTradeInfo( bool _isSeller,
                   const QString & _mwcAmount,
                   const QString & _secondaryAmount,
                   const QString & _secondaryCurrency,
                   const QString & _tradeId,
                   const QString & _state,
                   const QString & _initiatedTimeInterval,
                   const QString & _expirationTimeInterval,
                   const QString & _secondary_address) :
            isSeller(_isSeller), mwcAmount(_mwcAmount), secondaryAmount(_secondaryAmount), secondaryCurrency(_secondaryCurrency),
            tradeId(_tradeId), state(_state), initiatedTimeInterval(_initiatedTimeInterval), expirationTimeInterval(_expirationTimeInterval), secondary_address(_secondary_address) {}

    bool isValid() const {return !tradeId.isEmpty();}
    bool isDeletable() const;
    // Everything that can expire can be cancelled. No return steps doesn't have expiraiton time.
    bool isCancellable() const { return !expirationTimeInterval.isEmpty();}
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
    void sgnSwapTradesResult( QVector<QString> trades );
    void sgnDeleteSwapTrade(QString swapId, QString error);
    void sgnSwapTradeStatusUpdated(QString swapId, QString currentAction, QString currentState,
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


    void on_restoreTrade_clicked();

private:
    Ui::SwapList *ui;
    bridge::Swap * swap = nullptr;
    bridge::Config * config = nullptr;

    QVector<SwapTradeInfo> swapList;
    int swapTabSelection = 0; // 0 - 2 as shown in UI: incoming, outgoing, complete
};

}

#endif // S_SWAPLIST_W_H

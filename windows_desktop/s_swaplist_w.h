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

namespace Ui {
class SwapList;
}

namespace bridge {
class Swap;
}

namespace wnd {

struct SwapTradeInfo {
    QString info;
    QString tradeId;
    QString state;
    QString status;
    QString date;
    QString secondary_address;

    SwapTradeInfo() = default;
    SwapTradeInfo(const SwapTradeInfo & obj) = default;
    SwapTradeInfo & operator = (const SwapTradeInfo & obj) = default;

    SwapTradeInfo(const QString & _info, const QString & _tradeId,
                  const QString & _state,
                  const QString & _status,
                  const QString & _date,
                  const QString & _secondary_address) :
                  info(_info), tradeId(_tradeId), state(_state), status(_status), date(_date), secondary_address(_secondary_address) {}

    bool isValid() const {return !tradeId.isEmpty();}
    bool isDeletable() const;
    bool isRunnable() const;
    bool isStoppable() const;
};

class SwapList : public core::NavWnd {
Q_OBJECT
public:
    explicit SwapList(QWidget *parent);
    ~SwapList();

private:
    void updateButtons();
    void requestSwapList();
    SwapTradeInfo getSelectedTrade();
private slots:
    void sgnSwapTradesResult( QVector<QString> trades );
    void sgnDeleteSwapTrade(QString swapId, QString error);
    void sgnSwapTradeStatusUpdated(QString swapId, QString currentAction, QString currentState,
                                   QVector<QString> executionPlan,
                                   QVector<QString> tradeJournal);
    void sgnNewSwapTrade(QString currency, QString swapId);

    void on_tradeList_cellActivated(int row, int column);
    void on_tradeList_itemSelectionChanged();
    void on_newTradeBtn_clicked();
    void on_deleteBtn_clicked();
    void on_veiwTradeBtn_clicked();
    void on_refreshBtn_clicked();
    void on_showActiveBtn_clicked();
    void on_startButton_clicked();

    void on_stopButton_clicked();

private:
    Ui::SwapList *ui;
    bridge::Swap * swap = nullptr;
    bool isShowActiveOnly = false;

    QVector<SwapTradeInfo> swapList;
};

}

#endif // S_SWAPLIST_W_H

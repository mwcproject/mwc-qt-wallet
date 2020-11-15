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

#ifndef S_TRADEDETAILS_W_H
#define S_TRADEDETAILS_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class TradeDetails;
}

namespace bridge {
class Swap;
}

namespace wnd {

class TradeDetails : public core::NavWnd {
Q_OBJECT
public:
    explicit TradeDetails(QWidget *parent, QString swapId);
    ~TradeDetails();

private slots:
    void sgnRequestSwapDetails(QVector<QString> swapInfo,
                               QVector<QString> executionPlan,
                               QString currentAction,
                               QVector<QString> tradeJournal,
                               QString errMsg);

    void sgnSwapTradeStatusUpdated(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                                   int64_t expirationTime,
                                   QString lastProcessError,
                                   QVector<QString> executionPlan,
                                   QVector<QString> tradeJournal);

    void on_backButton_clicked();
    void on_refreshButton_clicked();

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void updateData(const QVector<QString> & executionPlan,
                   const QString & currentAction,
                   const QVector<QString> & tradeJournal);

private:
    Ui::TradeDetails *ui;
    bridge::Swap * swap = nullptr;
    QString swapId;
};

}

#endif // S_TRADEDETAILS_W_H

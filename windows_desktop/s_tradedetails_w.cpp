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

#include "s_tradedetails_w.h"
#include "ui_s_tradedetails_w.h"
#include "../bridge/swap_b.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

TradeDetails::TradeDetails(QWidget *parent, QString _swapId) :
        core::NavWnd(parent),
        ui(new Ui::TradeDetails),
        swapId(_swapId) {
    ui->setupUi(this);

    swap = new bridge::Swap(this);

    connect(swap, &bridge::Swap::sgnRequestTradeDetails, this, &TradeDetails::sgnRequestSwapDetails, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnSwapTradeStatusUpdated, this, &TradeDetails::sgnSwapTradeStatusUpdated, Qt::QueuedConnection);

    ui->progress->initLoader(true);

    ui->swapIdLabel->setText("SwapId: " + swapId);

    ui->executionPlanList->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->journalList->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    swap->requestTradeDetails(swapId);
}

TradeDetails::~TradeDetails() {
    delete ui;
}

void TradeDetails::updateData(const QVector<QString> & executionPlan,
                              const QString & currentAction,
                              const QVector<QString> & tradeJournal) {
    ui->executionPlanList->clearData();
    ui->journalList->clearData();
    ui->currentStatusLabel->setText("");

    Q_ASSERT(executionPlan.size()%3==0);
    for (int i=2; i<executionPlan.size(); i+=3) {
        bool active = (executionPlan[i-2] == "true");
        QString requred = executionPlan[i-1];
        QString stage = executionPlan[i];
        if (active) {
            stage = ">> " + stage;
        }
        else {
            stage = "   " + stage;
        }
        ui->executionPlanList->appendRow( QVector<QString>{ stage, requred }, active ? 1.0 : -1.0 );
    }

    ui->currentStatusLabel->setText("Current status: <b>" + currentAction + "</b>");

    Q_ASSERT(tradeJournal.size()%2==0);
    for (int i=1; i<tradeJournal.size(); i+=2) {
        QString message = tradeJournal[i-1];
        QString data = tradeJournal[i];
        ui->journalList->appendRow( QVector<QString>{data, message} );
    }
}


void TradeDetails::sgnRequestSwapDetails(QVector<QString> swapInfo,
                                         QVector<QString> executionPlan,
                                         QString currentAction,
                                         QVector<QString> tradeJournal,
                                         QString errMsg) {
    Q_ASSERT(swapInfo.size()>=1);
    QString reqSwapId = swapInfo[0];

    if (reqSwapId != swapId)
        return;

    ui->progress->hide();
    ui->executionPlanList->clearData();
    ui->journalList->clearData();
    ui->currentStatusLabel->setText("");

    if (!errMsg.isEmpty()) {
        Q_ASSERT(swapInfo.size()>=1);
        control::MessageBox::messageText( this, "Swap Trade details", "Unable to get details about the trade " + swapInfo[0] +
                    "\n\n" + errMsg );
        return;
    }

    updateData(executionPlan, currentAction, tradeJournal);
}

void TradeDetails::sgnSwapTradeStatusUpdated(QString reqSwapId, QString currentAction, QString currentState,
                               QVector<QString> executionPlan,
                               QVector<QString> tradeJournal) {
    Q_UNUSED(currentState);
    if (reqSwapId != swapId)
        return;

    updateData(executionPlan, currentAction, tradeJournal);
}

void TradeDetails::on_backButton_clicked() {
    swap->pageTradeList();
}

void wnd::TradeDetails::on_refreshButton_clicked() {
    ui->progress->show();
    swap->requestTradeDetails(swapId);
}

}

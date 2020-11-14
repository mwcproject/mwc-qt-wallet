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
#include "../dialogs_desktop/s_adjuststatedlg.h"
#include "../util_desktop/timeoutlock.h"

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

    setPageTitle("Trade: " + swapId);

    // Just want to match them.
    ui->executionPlan->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tradeJournal->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    swap->requestTradeDetails(swapId);
}

TradeDetails::~TradeDetails() {
    delete ui;
}

void TradeDetails::updateData(const QVector<QString> & executionPlan,
                              const QString & currentAction,
                              const QVector<QString> & tradeJournal) {

    // Execution plan...
    ui->executionPlan->clearAll();
    Q_ASSERT(executionPlan.size()%3==0);
    bool past = true;
    for (int i=2; i<executionPlan.size(); i+=3) {
        bool active = (executionPlan[i - 2] == "true");
        QString requred = executionPlan[i - 1];
        QString stage = executionPlan[i];

        // we have one space there
        requred.replace(" ", "  ");

        if (active)
            past = false;

        control::RichItem *itm = control::createMarkedItem(QString::number(i/3), ui->executionPlan, active,
                    control::LEFT_MARK_SIZE, control::LEFT_MARK_SIZE,
                    6, 8);

        itm->hbox()
            .addWidget(control::createLabel(itm, true, past, active ? (currentAction.isEmpty() ? stage : currentAction) : stage) )
            .addWidget(control::createLabel(itm, false, past, requred)).setFixedWidth(170)
            .pop();

        itm->apply();

        ui->executionPlan->addItem(itm);
    }
    ui->executionPlan->apply();

    // The trade journal
    ui->tradeJournal->clearAll();
    Q_ASSERT(tradeJournal.size()%2==0);
    for (int i=1; i<tradeJournal.size(); i+=2) {
        QString message = tradeJournal[i-1];
        QString data = tradeJournal[i];

        control::RichItem *itm = control::createMarkedItem(QString::number(i/3), ui->executionPlan, false,
                                                           control::LEFT_MARK_SIZE, control::LEFT_MARK_SIZE,
                                                           6, 8);

        itm->hbox()
                .addWidget(control::createLabel(itm, true, false, message))
                .addWidget(control::createLabel(itm, false, false, data)).setFixedWidth(170)
                .pop();
        itm->apply();

        ui->tradeJournal->addItem(itm);
    }
    ui->tradeJournal->apply();
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

    ui->executionPlan->clearAll();
    ui->tradeJournal->clearAll();

    if (!errMsg.isEmpty()) {
        Q_ASSERT(swapInfo.size()>=1);
        control::MessageBox::messageText( this, "Swap Trade details", "Unable to get details about the trade " + swapInfo[0] +
                    "\n\n" + errMsg );
        return;
    }

    updateData(executionPlan, currentAction, tradeJournal);
}

void TradeDetails::sgnSwapTradeStatusUpdated(QString reqSwapId, QString stateCmd, QString currentAction, QString currentState,
                               int64_t expirationTime,
                               QVector<QString> executionPlan,
                               QVector<QString> tradeJournal) {
    Q_UNUSED(currentState);
    Q_UNUSED(stateCmd);
    Q_UNUSED(expirationTime);

    if (reqSwapId != swapId)
        return;

    updateData(executionPlan, currentAction, tradeJournal);
}

void TradeDetails::on_backButton_clicked() {
    swap->pageTradeList();
}

void TradeDetails::on_refreshButton_clicked() {
    ui->progress->show();
    swap->requestTradeDetails(swapId);
}

void TradeDetails::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event)

    if(QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)){
        util::TimeoutLockObject("NewSwap3");

        dlg::AdjustStateDlg dlg(this, swapId);
        dlg.exec();
    }
}



}

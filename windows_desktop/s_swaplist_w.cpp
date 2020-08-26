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

#include "s_swaplist_w.h"
#include "ui_s_swaplist_w.h"
#include "../bridge/swap_b.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

///////////////////////////////////////////////////////////////////////
// SwapTradeInfo

bool SwapTradeInfo::isDeletable() const {
    if (!isValid())
        return false;

    return state.contains("was cancelled") || state.contains("successfully complete");
}

bool SwapTradeInfo::isRunnable() const {
    return isValid() && !isDeletable() && !isStoppable() && !secondary_address.isEmpty();
}

bool SwapTradeInfo::isStoppable() const {
    return isValid() && !status.startsWith("Not");
}

///////////////////////////////////////////////////////////////////////
// SwapList

SwapList::SwapList(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::SwapList) {
    ui->setupUi(this);

    ui->progress->initLoader(false);

    swap = new bridge::Swap(this);

    connect(swap, &bridge::Swap::sgnSwapTradesResult, this, &SwapList::sgnSwapTradesResult, Qt::QueuedConnection );
    connect(swap, &bridge::Swap::sgnDeleteSwapTrade, this, &SwapList::sgnDeleteSwapTrade, Qt::QueuedConnection );
    connect(swap, &bridge::Swap::sgnSwapTradeStatusUpdated, this, &SwapList::sgnSwapTradeStatusUpdated, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnNewSwapTrade, this, &SwapList::sgnNewSwapTrade, Qt::QueuedConnection);

    requestSwapList();
    updateButtons();
}

SwapList::~SwapList() {
    delete ui;
}

void SwapList::requestSwapList() {
    ui->progress->show();
    swap->requestSwapTrades();
}


void SwapList::updateButtons() {
    SwapTradeInfo selectedSwap = SwapList::getSelectedTrade();

    ui->deleteBtn->setEnabled( selectedSwap.isDeletable() );
    ui->veiwTradeBtn->setEnabled( selectedSwap.isValid() );
    ui->startButton->setEnabled(selectedSwap.isRunnable());
    ui->stopButton->setEnabled(selectedSwap.isStoppable());

    ui->showActiveBtn->setText( isShowActiveOnly ? "Show All" : "Show Active" );
}

SwapTradeInfo SwapList::getSelectedTrade() {
    int selectedIdx = ui->tradeList->getSelectedRow();
    SwapTradeInfo selectedSwap;
    if ( selectedIdx>=0 && selectedIdx<swapList.size() ) {
        selectedSwap = swapList[selectedIdx];
    }
    return selectedSwap;
}


void SwapList::on_tradeList_cellActivated(int row, int column) {
    Q_UNUSED(row)
    Q_UNUSED(column)
    // View seleted item
    on_veiwTradeBtn_clicked();
}

void SwapList::on_tradeList_itemSelectionChanged() {
    updateButtons();
}

void SwapList::on_newTradeBtn_clicked() {
    swap->startNewTrade();
}

void SwapList::on_deleteBtn_clicked() {
    SwapTradeInfo selectedSwap = SwapList::getSelectedTrade();
    if (selectedSwap.isDeletable()) {
        if (core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionText(
            this, "Swap Trade",
            "Are you sure that you want to delete the swap " + selectedSwap.tradeId + " from your records?",
            "No", "Yes",
            "Don't delete this swap record", "Yes, delete this swap record",
            false, true) ) {
            swap->deleteSwapTrade(selectedSwap.tradeId);
            requestSwapList();
        }
    }
}

void SwapList::on_veiwTradeBtn_clicked() {
    SwapTradeInfo selectedSwap = SwapList::getSelectedTrade();
    if (selectedSwap.isValid()) {
        swap->viewTrade(selectedSwap.tradeId);
    }
}

void SwapList::on_refreshBtn_clicked() {
    requestSwapList();
}

void SwapList::on_showActiveBtn_clicked() {
    isShowActiveOnly = !isShowActiveOnly;
    requestSwapList();
    updateButtons();
}

void SwapList::sgnSwapTradesResult( QVector<QString> trades ) {
    ui->progress->hide();
    swapList.clear();
    for (int i=5;i<trades.size(); i+=6) {
        SwapTradeInfo sti(trades[i-5],trades[i-4],trades[i-3],trades[i-2],trades[i-1],trades[i]);
        if (isShowActiveOnly && sti.isDeletable())
            continue;
        swapList.push_back(sti);
    }

    ui->tradeList->clearData();
    for (const auto & sw : swapList) {
        ui->tradeList->appendRow( QVector<QString>{ sw.info, sw.tradeId, sw.state, sw.status, sw.date } );
    }

    updateButtons();
}

void SwapList::sgnDeleteSwapTrade(QString swapId, QString error) {
    Q_UNUSED(swapId)
    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Delete Trade", "Unable to delete the trade " + swapId + "\n\n" + error);
    }
}

void SwapList::on_startButton_clicked()
{
    SwapTradeInfo selectedSwap = SwapList::getSelectedTrade();

    if (selectedSwap.isRunnable()) {
        swap->startAutoSwapTrade(selectedSwap.tradeId);
        requestSwapList();
    }
}

void SwapList::on_stopButton_clicked()
{
    SwapTradeInfo selectedSwap = SwapList::getSelectedTrade();

    if (selectedSwap.isStoppable()) {
        swap->stopAutoSwapTrade(selectedSwap.tradeId);
        requestSwapList();
    }
}

void SwapList::sgnSwapTradeStatusUpdated(QString swapId, QString currentAction, QString currentState,
                               QVector<QString> executionPlan,
                               QVector<QString> tradeJournal) {
    Q_UNUSED(executionPlan);
    Q_UNUSED(tradeJournal);

    for ( int i=0; i<swapList.size(); i++ ) {
        auto & sw = swapList[i];
        if (sw.tradeId == swapId) {
            // Updating this record
            if (swap->isRunning(swapId)) {
                sw.status = "Running";
                sw.state = currentAction;
            }
            else {
                sw.status = "Done";
                sw.state = currentState;
            }

            ui->tradeList->setItemText(i, 2, sw.state);
            ui->tradeList->setItemText(i, 3, sw.status);
            break;
        }
    }
}

void SwapList::sgnNewSwapTrade(QString currency, QString swapId) {
    Q_UNUSED(currency)
    Q_UNUSED(swapId)

    requestSwapList();
}



}

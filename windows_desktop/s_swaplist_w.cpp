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
#include "../bridge/config_b.h"
#include "../control_desktop/messagebox.h"
#include "../control_desktop/richvbox.h"
#include "../control_desktop/richitem.h"
#include <QFileDialog>

namespace wnd {

///////////////////////////////////////////////////////////////////////
// SwapTradeInfo

bool SwapTradeInfo::isDeletable() const {
    if (!isValid())
        return false;

    return state.contains("was cancelled") || state.contains("successfully complete");
}

///////////////////////////////////////////////////////////////////////
// SwapList

SwapList::SwapList(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::SwapList) {
    ui->setupUi(this);

    ui->progress->initLoader(false);

    swap = new bridge::Swap(this);
    config = new bridge::Config(this);

    connect(swap, &bridge::Swap::sgnSwapTradesResult, this, &SwapList::sgnSwapTradesResult, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnDeleteSwapTrade, this, &SwapList::sgnDeleteSwapTrade, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnSwapTradeStatusUpdated, this, &SwapList::sgnSwapTradeStatusUpdated,
            Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnNewSwapTrade, this, &SwapList::sgnNewSwapTrade, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnCancelTrade, this, &SwapList::sgnCancelTrade, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnBackupSwapTradeData, this, &SwapList::sgnBackupSwapTradeData, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnRestoreSwapTradeData, this, &SwapList::sgnRestoreSwapTradeData,
            Qt::QueuedConnection);

    connect(ui->swapsTable, &control::RichVBox::onItemActivated, this, &SwapList::onItemActivated,
            Qt::QueuedConnection);

    ui->checkEnforceBackup->setChecked(config->getSwapEnforceBackup());

    selectSwapTab(config->getSwapTabSelection());
}

SwapList::~SwapList() {
    delete ui;
}

void SwapList::selectSwapTab(int selection) {
    config->setSwapTabSelection(selection);
    swapTabSelection = selection;
    ui->incomingSwaps->setChecked(selection == 0);
    ui->outgoingSwaps->setChecked(selection == 1);
    ui->completedSwaps->setChecked(selection == 2);
    ui->restoreTradesTab->setChecked(selection == 3);
    if (selection == 3) {
        ui->restoreTradeHolder->show();
        ui->swapsTable->hide();
        ui->refreshBtnsHolder->hide();
    }
    else {
        ui->restoreTradeHolder->hide();
        ui->swapsTable->show();
        ui->refreshBtnsHolder->show();
    }
    // Requesting swap list in any case because that routing does counting and update the tabs with number of items.
    requestSwapList();
}


void SwapList::requestSwapList() {
    ui->progress->show();
    swap->requestSwapTrades();
}

void SwapList::onItemActivated(QString id) {
    swap->viewTrade(id);
}

void SwapList::on_newTradeButton_clicked() {
    swap->startNewTrade();
}

void SwapList::sgnSwapTradesResult(QVector<QString> trades) {
    ui->progress->hide();
    swapList.clear();
    // Result comes in series of 9 item tuples:
    // < <bool is Seller>, <mwcAmount>, <sec+amount>, <sec_currency>, <Trade Id>, <State>, <initiate_time_interval>, <expire_time_interval>  <secondary_address> >, ....
    for (int i = 8; i < trades.size(); i += 9) {
        SwapTradeInfo sti(trades[i - 8] == "true", trades[i - 7], trades[i - 6], trades[i - 5], trades[i - 4],
                          trades[i - 3], trades[i - 2], trades[i - 1], trades[i]);
        swapList.push_back(sti);
    }

    updateTradeListData();
}

void SwapList::updateTradeListData() {
    ui->swapsTable->clearAll();

    int incSwTrades = 0;
    int outSwTrades = 0;
    int compSwTrades = 0;

    for (const auto &sw : swapList) {

        if (sw.isDeletable()) {
            compSwTrades++;
        } else {
            if (sw.isSeller)
                outSwTrades++;
            else
                incSwTrades++;
        }

        switch (swapTabSelection) {
            case 0: // incoming swaps
                if (sw.isDeletable() || sw.isSeller)
                    continue;
                break;
            case 1:
                if (sw.isDeletable() || !sw.isSeller)
                    continue;
                break;
            case 2:
                if (!sw.isDeletable())
                    continue;
                break;
            default:
                continue; // It is not a list update, just counting the items and updating the tabs
        }


        // Must calculate the flag. We want to mark trades that
        // are close to expiration and require the backup...
        const bool marked = false;

        // Here we have only Hirizontal layout. I tis simple and swap related only.
        control::RichItem *itm = control::createMarkedItem(sw.tradeId, ui->swapsTable, marked);

        { // first line...
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);

            if (sw.isSeller) {
                itm->addWidget(
                        control::createIcon(itm, ":/img/iconSent@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
                itm->addWidget(control::createLabel(itm, false, false,
                                                    sw.mwcAmount + " MWC " + QChar(0x279E) + " " + sw.secondaryAmount +
                                                    " " + sw.secondaryCurrency));
            } else {
                itm->addWidget(
                        control::createIcon(itm, ":/img/iconReceived@2x.svg", control::ROW_HEIGHT,
                                            control::ROW_HEIGHT));
                itm->addWidget(control::createLabel(itm, false, false,
                                                    sw.secondaryAmount + " " + sw.secondaryCurrency + " " +
                                                    QChar(0x279E) + " " +
                                                    sw.mwcAmount + " MWC"));
            }
            itm->setMinWidth(250);
            itm->addWidget(control::createLabel(itm, false, true, sw.initiatedTimeInterval.isEmpty() ? "" :
                                                                  "initiated " + sw.initiatedTimeInterval +
                                                                  " ago")).setMinWidth(120);
            itm->addHSpacer();

            if (!sw.expirationTimeInterval.isEmpty())
                itm->addWidget(control::createLabel(itm, false, true,
                                                    "expires " + sw.initiatedTimeInterval + " ago")).setMinWidth(120);

            itm->pop();
        }

        // Second Line
        {
            itm->addFixedVSpacer(control::VBOX_SPACING); // add extra spacing
            itm->hbox();
            itm->addWidget(
                    control::createLabel(itm, false, true, "Trade ID: " + sw.tradeId, control::FONT_SMALL)).setMinWidth(
                    350);
            itm->pop();
        }
        itm->addWidget(control::createLabel(itm, true, false, "Status: " + sw.state, control::FONT_SMALL));

        // Buttons need to go in full size.
        // So we need to finish the main vertical layout
        itm->pop();

        // Currently root Horz layout is current one.
        // Adding another vertical layout with a single horiz line & spacer at the bottom.
        itm->vbox().setContentsMargins(0, control::VBOX_MARGIN, 0, 0).setSpacing(control::VBOX_SPACING);

        const int BTN_FONT_SIZE = 13;
        const int BTN_WIDTH = 80;

        if (sw.isCancellable()) {
            itm->addWidget(
                    (new control::RichButton(itm, "Cancel", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                             "Cancel this swap Trade", BTN_FONT_SIZE))->
                            setCallback(this, "Cancel:" + sw.tradeId));
        }

        if (sw.isDeletable()) {
            itm->addWidget(
                    (new control::RichButton(itm, "Delete", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                             "Cancel this swap Trade", BTN_FONT_SIZE))->
                            setCallback(this, "Delete:" + sw.tradeId));
        }

        if (!sw.isSeller) {
            // Buyer can backup immediatelly.
            itm->addWidget((new control::RichButton(itm, "Backup", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                                    "Backup your trade data, so you will be able to get a refund in case of the hardware failure",
                                                    BTN_FONT_SIZE))->
                    setCallback(this, "Backup:" + sw.tradeId));
        }

        // Check if this Trade is not running, it is mean it need to be accepted to start
        if (!sw.isDeletable() && !swap->isRunning(sw.tradeId)) {
            itm->addWidget((new control::RichButton(itm, "Accept", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                                    "Review and accept this swap Trade", BTN_FONT_SIZE))->
                    setCallback(this, "Accept:" + sw.tradeId));
        }

        itm->addVSpacer();

        ui->swapsTable->addItem(itm);
    }
    ui->swapsTable->apply();

    // Updating the counters for the tabs
    ui->incomingSwaps->setText("Incoming Swaps (" + QString::number(incSwTrades) + ")");
    ui->outgoingSwaps->setText("Outgoing Swaps (" + QString::number(outSwTrades) + ")");
    ui->completedSwaps->setText("Completed Swaps (" + QString::number(compSwTrades) + ")");
}

void SwapList::richButtonPressed(control::RichButton *button, QString coockie) {
    Q_UNUSED(button);

    QStringList dt = coockie.split(':');
    if (dt.size() != 2) {
        Q_ASSERT(false);
        return;
    }

    QString cmd = dt[0];
    QString tradeId = dt[1];

    if (cmd == "Cancel") {
        if (core::WndManager::RETURN_CODE::BTN1 == control::MessageBox::questionText(this, "Warning",
                                                                                     "Are you sure you want to cancel this trade? Please note, that refund process might take time and your wallet need to be online to do that. "
                                                                                     "If your wallet will not be online until the swap process will be finished, you might lost the funds.",
                                                                                     "No", "Yes",
                                                                                     "Continue this swap trade",
                                                                                     "Cancel the trade and get a refund if needed",
                                                                                     true, false))
            return;

        ui->progress->show();
        swap->cancelTrade(tradeId);
    } else if (cmd == "Delete") {
        if (core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionText(
                this, "Swap Trade",
                "Are you sure that you want to delete the swap " + tradeId + " from your records?",
                "No", "Yes",
                "Don't delete this swap record", "Yes, delete this swap record",
                false, true)) {
            ui->progress->show();
            swap->deleteSwapTrade(tradeId);
        }
    } else if (cmd == "Backup") {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Backup the trade Data"),
                                                        config->getPathFor("SwapTrades"),
                                                        tr("MWC Swap Trade (*.trade)"));

        if (fileName.length() == 0)
            return;

        auto fileOk = util::validateMwc713Str(fileName);
        if (!fileOk.first) {
            core::getWndManager()->messageTextDlg("File Path",
                                                  "This file path is not acceptable.\n" + fileOk.second);
            return;
        }

        if (!fileName.endsWith(".trade"))
            fileName += ".trade";

        // Update path
        QFileInfo flInfo(fileName);
        config->updatePathFor("SwapTrades", flInfo.path());

        // Requesting export from the wallet
        swap->backupSwapTradeData(tradeId, fileName);
        ui->progress->show();
    } else if (cmd == "Accept") {
        // Accept mean that user need to review the deal. I tis not implemented now, waiting for Brent. We might just go with a modal for that.
        swap->viewTrade(tradeId);
    } else {
        Q_ASSERT(false);
    }
}

void SwapList::on_restoreTradeBtn_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Restore the trade Data"),
                                                    config->getPathFor("SwapTrades"),
                                                    tr("MWC Swap Trade (*.trade)"));

    if (fileName.length() == 0)
        return;

    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return;
    }

    QFileInfo flInfo(fileName);
    config->updatePathFor("SwapTrades", flInfo.path());

    // Requesting export from the wallet
    swap->restoreSwapTradeData(fileName);
    ui->progress->show();
}

void SwapList::sgnCancelTrade(QString swId, QString error) {
    ui->progress->hide();

    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Trade cancellation",
                                         "Unable to cancel the swap " + swId + "\n\n" + error);
        return;
    }
    on_refreshButton_clicked();
}


void SwapList::sgnDeleteSwapTrade(QString swapId, QString error) {
    Q_UNUSED(swapId)
    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Delete Trade", "Unable to delete the trade " + swapId + "\n\n" + error);
    }
    on_refreshButton_clicked();
}

void SwapList::sgnSwapTradeStatusUpdated(QString swapId, QString currentAction, QString currentState,
                                         QVector<QString> executionPlan,
                                         QVector<QString> tradeJournal) {
    Q_UNUSED(swapId)
    Q_UNUSED(currentAction)
    Q_UNUSED(currentState)
    Q_UNUSED(executionPlan);
    Q_UNUSED(tradeJournal);

    // need to implement. There are warnings, KEEP THEM as TODO

/*    for (int i = 0; i < swapList.size(); i++) {
        auto &sw = swapList[i];
        if (sw.tradeId == swapId) {
            // Updating this record
            if (swap->isRunning(swapId)) {
                sw.status = "Running";
                sw.state = currentAction;
            } else {
                sw.status = "Done";
                sw.state = currentState;
            }

            ui->tradeList->setItemText(i, 2, sw.state);
            ui->tradeList->setItemText(i, 3, sw.status);
            break;
        }
    }*/
}

void SwapList::sgnNewSwapTrade(QString currency, QString swapId) {
    Q_UNUSED(currency)
    Q_UNUSED(swapId)

    requestSwapList();
}

// Respond from backupSwapTradeData
// On OK will get exportedFileName
void SwapList::sgnBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage) {
    ui->progress->hide();

    if (errorMessage.length() > 0) {
        control::MessageBox::messageText(this, "Backup Error",
                                         "Unable to backup the trade " + swapId + "\n\n" + errorMessage);
        return;
    }

    control::MessageBox::messageText(this, "Backup",
                                     "Your trade " + swapId + " is exported at the file\n" + exportedFileName +
                                     "\n\nPlease keep it on your backup drive until this trade will be finished.");
}

void SwapList::sgnRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage) {
    ui->progress->hide();

    if (errorMessage.length() > 0) {
        control::MessageBox::messageText(this, "Restore Error",
                                         "Unable to restore the trade from the file\n" + importedFilename + "\n\n" +
                                         errorMessage);
        return;
    }

    on_refreshButton_clicked();
    control::MessageBox::messageText(this, "Restore",
                                     "Your trade " + swapId + " is successfully restored.");
}

void SwapList::on_outgoingSwaps_clicked() {
    selectSwapTab(1);
}

void SwapList::on_incomingSwaps_clicked() {
    selectSwapTab(0);
}

void SwapList::on_completedSwaps_clicked() {
    selectSwapTab(2);
}

void SwapList::on_refreshButton_clicked() {
    requestSwapList();
}

void SwapList::on_restoreTradesTab_clicked() {
    selectSwapTab(3);
}

void SwapList::on_checkEnforceBackup_clicked() {
    config->setSwapEnforceBackup(ui->checkEnforceBackup->isChecked());
}

}

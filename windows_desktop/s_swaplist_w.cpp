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
#include "../bridge/wnd/swap_b.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../control_desktop/messagebox.h"
#include "../control_desktop/richvbox.h"
#include "../control_desktop/richitem.h"
#include <QSet>
#include <QFileDialog>

namespace wnd {

// Update current state and UI
void SwapTradeInfo::updateData(QString _stateCmd, QString _status, QString _lastProcessError, int64_t _expirationTime, int swapTabSelection,
                               bridge::Util * util, bridge::Config * config, bridge::Swap * swap) {
    if (!_stateCmd.isEmpty())
        stateCmd = _stateCmd;

    if (!_status.isEmpty())
        status = _status;

    if (_lastProcessError.isEmpty() || _expirationTime>0)
        expirationTime = _expirationTime;

    lastProcessError = _lastProcessError;

    Q_ASSERT(util);

    applyState2Ui(util, config, swap, swapTabSelection);
}

void SwapTradeInfo::applyState2Ui(bridge::Util * util, bridge::Config * config, bridge::Swap * swap, int swapTabSelection) {
    if (initTimeLable == nullptr)
        return; // All null

    int64_t timestampSec = QDateTime::currentSecsSinceEpoch();

    Q_ASSERT(initTimeLable);
    if (initiatedTime>0) {
        if (swapTabSelection==2) {
            // For completed let's show the date
            QDateTime iniTime = QDateTime::fromSecsSinceEpoch( initiatedTime, Qt::TimeSpec::LocalTime);
            initTimeLable->setText(iniTime.toString("MMM d, yyyy / H:mmap"));
        }
        else {
            initTimeLable->setText("initiated " + util->interval2String(timestampSec - initiatedTime, false) + " ago");
        }
    }
    else
        initTimeLable->setText("");

    Q_ASSERT( expirationLable );
    if (expirationTime>0) {
        expirationLable->show();
        expirationLable->setText( "expires in " + util->interval2String( expirationTime - timestampSec, false ) );
    }
    else {
        expirationLable->hide();
    }

    Q_ASSERT(statusLable);
    statusLable->setText("Status: " + status);

    Q_ASSERT(lastErrorLable);
    lastErrorLable->setText( lastProcessError.isEmpty() ? "" : "Error: " + lastProcessError);
    if (lastProcessError.isEmpty()) {
        lastErrorLable->hide();
    }
    else {
        lastErrorLable->show();
    }

    Q_ASSERT(cancelBtn);
    if ( swap->isSwapCancellable(stateCmd))
        cancelBtn->show();
    else
        cancelBtn->hide();

    Q_ASSERT(deleteBtn);
    if ( swap->isSwapDone(stateCmd))
        deleteBtn->show();
    else
        deleteBtn->hide();

    Q_ASSERT(acceptBtn);
    bool hasAccept = false;
    if (!config->isTradeAccepted(tradeId) && swap->isSwapWatingToAccept(stateCmd)) {
        acceptBtn->show();
        statusLable->setText("Status: Not running. Please accept the offer first.");
        hasAccept = true;
    }
    else {
        acceptBtn->hide();
    }

    Q_ASSERT(backupBtn);
    int doneBackup = config->getSwapBackStatus(tradeId);
    int swBackup = swap->getSwapBackup(stateCmd);
    // We don't want to see Accpept and backup together. Even it works, there is no enough space in the UI.
    if (swBackup > doneBackup && !hasAccept)
        backupBtn->show();
    else
        backupBtn->hide();

    Q_ASSERT(markWnd);
    if ( config->getMaxBackupStatus(tradeId, swBackup) >=2 || !lastProcessError.isEmpty() )
        markWnd->setStyleSheet(control::LEFT_MARK_ON);
    else
        markWnd->setStyleSheet(control::LEFT_MARK_OFF);

    Q_ASSERT(noteLabel);
    QString noteStr = config->getSwapNote(tradeId);
    noteLabel->setText(noteStr);
    if (noteStr.isEmpty())
        noteLabel->hide();
    else
        noteLabel->show();
}


///////////////////////////////////////////////////////////////////////
// SwapList

SwapList::SwapList(QWidget *parent, bool selectIncoming, bool selectOutgoing, bool selectBackup) :
        core::NavWnd(parent),
        ui(new Ui::SwapList) {
    ui->setupUi(this);

    ui->progress->initLoader(false);

    swap = new bridge::Swap(this);
    config = new bridge::Config(this);
    util = new bridge::Util(this);

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

    //ui->checkEnforceBackup->setChecked(config->getSwapEnforceBackup());
    ui->swapBackupDir->setText( config->getSwapBackupDir() );

    int tabSelection = config->getSwapTabSelection();
    if (selectIncoming)
        tabSelection = 0;
    else if (selectOutgoing)
        tabSelection = 1;
    else if (selectBackup)
        tabSelection = 3;

    selectSwapTab(tabSelection);
}

SwapList::~SwapList() {
    clearSwapList();
    delete ui;
    ui=nullptr;
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

        clearSwapList();
        ui->swapsTable->clearAll();

    }
    // Requesting swap list in any case because that routing does counting and update the tabs with number of items.
    requestSwapList();
}


void SwapList::requestSwapList() {
    ui->progress->show();
    swap->requestSwapTrades("SwapListWnd");
}

void SwapList::onItemActivated(QString id) {

    for (  const SwapTradeInfo & sw : swapList ) {
        if (id == sw.tradeId) {
            swap->viewTrade(id, sw.stateCmd);
            break;
        }
    }
}

void SwapList::on_newTradeButton_clicked() {
    swap->initiateNewTrade();
}

void SwapList::sgnSwapTradesResult(QString cookie, QVector<QString> trades, QString error) {
    if (cookie != "SwapListWnd")
        return;

    ui->progress->hide();
    clearSwapList();

    // Result comes in series of 11 item tuples:
    // < <bool is Seller>, <mwcAmount>, <sec+amount>, <sec_currency>, <Trade Id>, <State>, <initiate_time_interval>, <expire_time_interval>  <secondary_address> <last_process_error> >, ....
    for (int i = 10; i < trades.size(); i += 11) {
        SwapTradeInfo sti(trades[i - 10] == "true", trades[i - 9], trades[i - 8], trades[i - 7], trades[i - 6], trades[i - 5],
                          trades[i - 4], trades[i - 3].toLongLong(), trades[i - 2].toLongLong(), trades[i-1], trades[i]);
        swapList.push_back(sti);
    }

    updateTradeListData();

    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Error", "Unable to request a list of Swap trades.\n\n" + error);
    }
}

void SwapList::clearSwapList() {
    for (auto &sw : swapList) {
        if (sw.cancelBtn)
            sw.cancelBtn->setCallback(nullptr, "");
        if (sw.deleteBtn)
            sw.deleteBtn->setCallback(nullptr, "");
        if (sw.backupBtn)
            sw.backupBtn->setCallback(nullptr, "");
        if (sw.acceptBtn)
            sw.acceptBtn->setCallback(nullptr, "");
    }

    swapList.clear();
}


void SwapList::updateTradeListData() {
    ui->swapsTable->clearAll();
    for (auto &sw : swapList)
        sw.resetUI();

    int incSwTrades = 0;
    int outSwTrades = 0;
    int compSwTrades = 0;

    for (auto &sw : swapList) {

        bool done = swap->isSwapDone(sw.stateCmd);

        if (done) {
            compSwTrades++;
        } else {
            if (sw.isSeller)
                outSwTrades++;
            else
                incSwTrades++;
        }

        switch (swapTabSelection) {
            case 0: // incoming swaps
                if (done || sw.isSeller)
                    continue;
                break;
            case 1:
                if (done || !sw.isSeller)
                    continue;
                break;
            case 2:
                if (!done)
                    continue;
                break;
            default:
                continue; // It is not a list update, just counting the items and updating the tabs
        }


        // Must calculate the flag. We want to mark trades that
        // are close to expiration and require the backup...
        const bool marked = false;

        // Here we have only Hirizontal layout. It is simple and swap related only.
        control::RichItem *itm = control::createMarkedItem(sw.tradeId, ui->swapsTable, marked);

        sw.markWnd = itm->getCurrentWidget();

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
            sw.initTimeLable = (QLabel*) itm->addWidget(control::createLabel(itm, false, true, "")).setMinWidth(120).getCurrentWidget();
            itm->addHSpacer();
            sw.expirationLable = (QLabel*) itm->addWidget(control::createLabel(itm, false, true,"")).setMinWidth(120).getCurrentWidget();
            itm->pop();
        }

        // Second Line
        {
            itm->addFixedVSpacer(control::VBOX_SPACING); // add extra spacing
            itm->hbox().setContentsMargins(0, 0, 0, 0);
            itm->addWidget(
                    control::createLabel(itm, false, true, "Trade ID: " + sw.tradeId, control::FONT_SMALL)).setMinWidth(
                    350);
            itm->addHSpacer().pop();
        }
        {
            itm->hbox().setContentsMargins(0, 0, 0, 0);
            sw.statusLable = (QLabel *) itm->addWidget(
                    control::createLabel(itm, false, false, "", control::FONT_SMALL)).getCurrentWidget();
            itm->addHSpacer().pop();
        }
        {
            itm->hbox().setContentsMargins(0, 0, 0, 0);
            sw.noteLabel = (QLabel *) itm->addWidget(
                    control::createLabel(itm, true, false, "")).getCurrentWidget();
            itm->pop();
            sw.noteLabel->hide();
        }

        sw.lastErrorLable = (QLabel*) itm->addWidget(control::createLabel(itm, true, false, "", control::FONT_NORMAL, "#CCFF33")).getCurrentWidget();

        // Buttons need to go in full size.
        // So we need to finish the main vertical layout
        itm->pop();

        // Currently root Horz layout is current one.
        // Adding another vertical layout with a single horiz line & spacer at the bottom.
        itm->vbox().setContentsMargins(0, control::VBOX_MARGIN, 0, 0).setSpacing(control::VBOX_SPACING);

        const int BTN_FONT_SIZE = 13;
        const int BTN_WIDTH = 80;

        sw.cancelBtn = (control::RichButton *) itm->addWidget(
                    (new control::RichButton(itm, "Cancel", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                             "Cancel this swap Trade", BTN_FONT_SIZE))->
                            setCallback(this, "Cancel:" + sw.tradeId)).getCurrentWidget();

        sw.deleteBtn = (control::RichButton *) itm->addWidget(
                (new control::RichButton(itm, "Delete", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                         "Cancel this swap Trade", BTN_FONT_SIZE))->
                        setCallback(this, "Delete:" + sw.tradeId)).getCurrentWidget();

        sw.backupBtn = (control::RichButton *) itm->addWidget((new control::RichButton(itm, "Backup", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                                "Backup your trade data, so you will be able to get a refund in case of the hardware failure",
                                                BTN_FONT_SIZE))->
                setCallback(this, "Backup:" + sw.tradeId + ":"+sw.stateCmd)).getCurrentWidget();

        sw.acceptBtn = (control::RichButton *) itm->addWidget((new control::RichButton(itm, "Accept", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                                "Review and accept this swap Trade", BTN_FONT_SIZE))->
                setCallback(this, "Accept:" + sw.tradeId + ":"+sw.stateCmd)).getCurrentWidget();

        itm->addVSpacer();

        ui->swapsTable->addItem(itm);

        sw.applyState2Ui(util, config, swap, swapTabSelection);
    }
    ui->swapsTable->apply();

    // Updating the counters for the tabs
    ui->incomingSwaps->setText("Incoming Swaps (" + QString::number(incSwTrades) + ")");
    ui->outgoingSwaps->setText("Outgoing Swaps (" + QString::number(outSwTrades) + ")");
    ui->completedSwaps->setText("Completed / Cancelled (" + QString::number(compSwTrades) + ")");
}

void SwapList::richButtonPressed(control::RichButton *button, QString cookie) {
    QStringList dt = cookie.split(':');
    if (dt.size() < 2) {
        Q_ASSERT(false);
        return;
    }

    QString cmd = dt[0];
    QString tradeId = dt[1];

    if (cmd == "Cancel") {
        button->hide();
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
        button->hide();
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
        QString fileName = util->getSaveFileName("Backup the trade Data",
                                                        "SwapTrades",
                                                        "MWC Swap Trade (*.trade)", ".trade");

        if (fileName.isEmpty())
            return;

        swapBackupInProgress = true;

        // Requesting export from the wallet
        swap->backupSwapTradeData(tradeId, fileName);

        Q_ASSERT( dt.size() == 3);
        QString stateCmd = dt[2];
        config->setSwapBackStatus(tradeId, swap->getSwapBackup(stateCmd));

        ui->progress->show();
    } else if (cmd == "Accept") {
        // Accept mean that user need to review the deal. I tis not implemented now, waiting for Brent. We might just go with a modal for that.
        Q_ASSERT( dt.size() == 3);
        QString stateCmd = dt[2];
        swap->viewTrade(tradeId, stateCmd);
    } else {
        Q_ASSERT(false);
    }
}

void SwapList::on_restoreTradeBtn_clicked() {
    QString fileName = util->getOpenFileName("Restore the trade Data",
                                                    "SwapTrades",
                                                    "MWC Swap Trade (*.trade)");
    if ( fileName.isEmpty() )
        return;

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

void SwapList::sgnSwapTradeStatusUpdated(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                                         int64_t expirationTime,
                                         QString lastProcessError,
                                         QVector<QString> executionPlan,
                                         QVector<QString> tradeJournal) {
    Q_UNUSED(swapId)
    Q_UNUSED(currentAction)
    Q_UNUSED(currentState)
    Q_UNUSED(executionPlan);
    Q_UNUSED(tradeJournal);

    // need to implement. There are warnings, KEEP THEM as TODO

    for (int i = 0; i < swapList.size(); i++) {
        auto &sw = swapList[i];
        if (sw.tradeId == swapId) {
            // Updating this record
            sw.updateData(stateCmd, currentAction.isEmpty() ? currentState : currentAction,
                          lastProcessError, expirationTime, swapTabSelection,
                          util, config, swap);
            break;
        }
    }
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

    if (!swapBackupInProgress)
        return;

    swapBackupInProgress = false;

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
    Q_UNUSED(swapId)
    ui->progress->hide();

    if (errorMessage.length() > 0) {
        control::MessageBox::messageText(this, "Restore Error",
                                         "Unable to restore the trade from the file\n" + importedFilename + "\n\n" +
                                         errorMessage);
        return;
    }

    on_refreshButton_clicked();
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

void SwapList::on_selectBackupDirBtn_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this,
            "Select trade backup directory");
    if (dir.isEmpty())
        return;

    ui->swapBackupDir->setText(dir);
    config->setSwapBackupDir(dir);
}

void SwapList::on_swapBackupDir_textEdited(const QString &dir) {
    config->setSwapBackupDir(dir);
}

}

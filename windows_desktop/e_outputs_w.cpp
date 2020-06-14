// Copyright 2019 The MWC Developers
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

#include "e_outputs_w.h"
#include "ui_e_outputs.h"
#include <QDebug>
#include <control_desktop/messagebox.h>
#include "../dialogs_desktop/e_showoutputdlg.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/hodlstatus_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/wnd/e_outputs_b.h"

namespace wnd {

const int LOCK_OUTPUT_COLUMN_IDX = 3;

// static
bool Outputs::lockMessageWasShown = false;

Outputs::Outputs(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::Outputs) {
    ui->setupUi(this);

    config = new bridge::Config(this);
    hodlStatus = new bridge::HodlStatus(this);
    wallet = new bridge::Wallet(this);
    outputs = new bridge::Outputs(this);

    QObject::connect( wallet, &bridge::Wallet::sgnOutputs,
                      this, &Outputs::onSgnOutputs, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
                      this, &Outputs::onSgnWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnNewNotificationMessage,
                      this, &Outputs::onSgnNewNotificationMessage, Qt::QueuedConnection);

    ui->outputsTable->setHightlightColors(QColor(255, 255, 255, 51), QColor(255, 255, 255, 153)); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->outputsTable->setStripeAlfaDelta(5); // very small number

    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    bool showAll = config->isShowOutputAll();
    ui->showAll->setEnabled(!showAll); // inverse state, enabled is a switch
    ui->showUnspent->setEnabled(showAll);

    QString accName = updateAccountsData();

    inHodl = hodlStatus->isInHodl();
    canLockOutputs = config->isLockOutputEnabled();

    initTableHeaders();

    requestOutputs(accName);
}

void Outputs::panelWndStarted() {
    ui->outputsTable->setFocus();
}


Outputs::~Outputs() {
    config->setShowOutputAll( isShowUnspent() );
    saveTableHeaders();
    delete ui;
}

void Outputs::initTableHeaders() {

    tableId = "Outputs_N";
    QVector<QString> columns{"TX #", "MWC", "STATUS", "CONF", "COMMITMENT", "CB", "HEIGHT", "LOCK H" };
    QVector<int> widths{40, 90, 100, 70, 240, 50, 70, 70};

    if (canLockOutputs) {
        columns.insert(LOCK_OUTPUT_COLUMN_IDX, "LOCKED");
        widths.insert(LOCK_OUTPUT_COLUMN_IDX, 60);
        tableId += "L";

        ui->outputsTable->addHighlightedColumn(LOCK_OUTPUT_COLUMN_IDX);
    }

    if (inHodl) {
        columns.push_back("HODL");
        widths.push_back(60);
        tableId += "H";
    }

    QVector<int> ww = config->getColumnsWidhts( tableId );
    if (ww.size() == widths.size()) {
        widths = ww;
    }

    ui->outputsTable->setColumnCount(widths.size());
    for (int t=0; t<widths.size(); t++) {
        ui->outputsTable->setColumnWidth(t, widths[t]);
        QTableWidgetItem * itm = new QTableWidgetItem( columns[t]) ;
        ui->outputsTable->setHorizontalHeaderItem( t, itm );
    }
}

void Outputs::saveTableHeaders() {
    QVector<int>  width = ui->outputsTable->getColumnWidths();
    config->updateColumnsWidhts( tableId, width );
}

int Outputs::calcPageSize() const {
    QSize sz1 = ui->outputsTable->size();
    QSize sz2 = ui->progressFrame->size();

    return ListWithColumns::getNumberOfVisibleRows(std::max(sz1.height(), std::max(0, sz2.height() - 50)));
}


void Outputs::on_prevBtn_clicked() {
    if (currentPagePosition > 0) {
        int pageSize = calcPageSize();
        currentPagePosition = std::max( 0, currentPagePosition-pageSize );
        updateShownData();
    }
}

void Outputs::on_nextBtn_clicked() {
    if (currentPagePosition + shownData.size() < allData.size()) {
        int pageSize = calcPageSize();
        currentPagePosition = std::min( allData.size()-pageSize, currentPagePosition+pageSize );
        updateShownData();
    }
}

void Outputs::updateShownData() {
    int pageSize = calcPageSize();
    shownData.clear();
    ui->outputsTable->clearData();
    if (currentPagePosition < 0 || allData.isEmpty() || pageSize <= 0) {
        ui->nextBtn->setEnabled(false);
        ui->prevBtn->setEnabled(false);
        ui->pageLabel->setText("");
        return;
    } else {
        int total = allData.size();

        ui->nextBtn->setEnabled(currentPagePosition < total - pageSize);
        ui->prevBtn->setEnabled(currentPagePosition > 0);


        if (total <= 1) {
            ui->pageLabel->setText(QString::number(total) +
                                   " of " + QString::number(total));
        } else {
            ui->pageLabel->setText(QString::number(currentPagePosition + 1) + "-" +
                                   QString::number(std::min(currentPagePosition + pageSize - 1 + 1, total)) +
                                   " of " + QString::number(total));
        }

        for (int i=currentPagePosition; i<total && pageSize>0; i++, pageSize--) {
            shownData.push_back(allData[i]);
        }

        ui->outputsTable->clearData();

        qDebug() << "updating output table for " << shownData.size() << " rows";
        int row = 0;
        for (int i = shownData.size()-1; i >= 0; i--) {
            auto &out = shownData[i];

            QVector<QString> rowData{
                    QString::number(out.txIdx + 1),
                    // out.status, // Status allways 'unspent', so no reasons to print it.
                    util::nano2one(out.valueNano),
                    out.status,
                    out.numOfConfirms,
                    out.outputCommitment,
                    out.coinbase ? "Yes" : "No",
                    out.blockHeight,
                    out.lockedUntil
            };

            if (canLockOutputs) {
                rowData.insert(LOCK_OUTPUT_COLUMN_IDX, "" );
            }

            if (inHodl) {
                rowData.push_back( hodlStatus->getOutputHodlStatus(out.outputCommitment) );
            }

            ui->outputsTable->appendRow( rowData );

            showLockedState(row++, out);
        }
    }
}

QString Outputs::currentSelectedAccount() {
    return ui->accountComboBox->currentData().toString();
}

void Outputs::onSgnOutputs( QString account, bool showSpent, QString height, QVector<QString> outputs) {
    Q_UNUSED(height);

    if (account != currentSelectedAccount() || showSpent != isShowUnspent())
        return;

    ui->progressFrame->hide();
    ui->tableFrame->show();

    allData.clear();
    shownData.clear();

    for (const QString & s : outputs) {
        allData.push_back( wallet::WalletOutput::fromJson(s) );
    }

    int pageSize = calcPageSize();
    currentPagePosition = std::max(0, allData.size() - pageSize);

    updateShownData();
}

void Outputs::on_refreshButton_clicked() {
    requestOutputs(currentSelectedAccount());
}

// Request and reset page counter
void Outputs::requestOutputs(QString account) {
    currentPagePosition = INT_MAX; // Reset Paging
    allData.clear();
    shownData.clear();

    ui->progressFrame->show();
    ui->tableFrame->hide();

    updateShownData();

    wallet->requestOutputs(account, isShowUnspent(), true);
}

void Outputs::on_accountComboBox_activated(int index) {
    Q_UNUSED(index)
    requestOutputs(currentSelectedAccount());
}

void Outputs::onSgnWalletBalanceUpdated() {
    updateAccountsData();
}

QString Outputs::updateAccountsData() {

    QVector<QString> accounts = wallet->getWalletBalance(true,false,true);
    QString selectedAccount = wallet->getCurrentAccountName();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx = 0;
    for (int i=1; i<accounts.size(); i+=2) {
        if (accounts[i-1] == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem(accounts[i], accounts[i-1]);
        idx++;
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
    return currentSelectedAccount();
}


void Outputs::on_showAll_clicked() {
    ui->showAll->setEnabled(false);
    ui->showUnspent->setEnabled(true);
    on_refreshButton_clicked();
}

void Outputs::on_showUnspent_clicked() {
    ui->showAll->setEnabled(true);
    ui->showUnspent->setEnabled(false);
    on_refreshButton_clicked();
}

bool Outputs::isShowUnspent() const {
    return !ui->showAll->isEnabled();
}

// return null if nothing was selected
wallet::WalletOutput * Outputs::getSelectedOutput() {
    int row = ui->outputsTable->getSelectedRow();
    if (row<0 || row>=shownData.size())
        return nullptr;

    return &shownData[shownData.size()-1-row];
}

void Outputs::on_outputsTable_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    util::TimeoutLockObject to( "Outputs");
    wallet::WalletOutput * selected = getSelectedOutput();

    if (selected==nullptr)
        return;

    wallet::WalletOutput out = *selected;

    bool locked = config->isLockedOutput(out.outputCommitment);

    QString account = currentSelectedAccount();
    QString outputNote = config->getOutputNote( out.outputCommitment);
    dlg::ShowOutputDlg showOutputDlg(this, out,
                                     outputNote,
                                     config->isLockOutputEnabled(), locked );
    connect(&showOutputDlg, &dlg::ShowOutputDlg::saveOutputNote, this, &Outputs::saveOutputNote);
    if (showOutputDlg.exec() == QDialog::Accepted) {
        if (locked != showOutputDlg.isLocked()) {
            if (showLockMessage()) {
                // Updating the state
                config->setLockedOutput(showOutputDlg.isLocked(), out.outputCommitment);
                showLockedState(row, out);
            }
        }
    }
}

void Outputs::on_outputsTable_cellClicked(int row, int column)
{
    if (!canLockOutputs)
        return;

    // We can change the lock flag
    if (column == LOCK_OUTPUT_COLUMN_IDX) {
        wallet::WalletOutput * selected = getSelectedOutput();
        if (selected==nullptr)
            return;

        if (!selected->isUnspent())
            return;

        wallet::WalletOutput out = *selected;

        if (showLockMessage()) {
            bool locked = !config->isLockedOutput(out.outputCommitment);
            config->setLockedOutput(locked, out.outputCommitment);
            showLockedState(row, out);
        }
    }
}

void Outputs::showLockedState(int row, const wallet::WalletOutput & output) {
    if (!canLockOutputs)
        return;

    QString lockState = "N/A";
    if (output.isUnspent()) {
            lockState = config->isLockedOutput(output.outputCommitment) ? "YES" : "NO";
    }
    ui->outputsTable->setItemText(row,LOCK_OUTPUT_COLUMN_IDX, lockState);
}

void Outputs::saveOutputNote( QString commitment, QString note) {
    if (note.isEmpty()) {
        config->deleteOutputNote(commitment);
    }
    else {
        // add new note or update existing note for this commitment
        config->updateOutputNote(commitment, note);
    }
}

// return true if user fine with lock changes
bool Outputs::showLockMessage() {
    if (lockMessageWasShown)
        return true;

    if ( core::WndManager::RETURN_CODE::BTN2 != control::MessageBox::questionText(this, "Locking Output",
            "By manually locking output you are preventing it from spending by QT wallet.\nLocked outputs amount will be shown as Locked balance until you change this.",
            "Cancel", "Continue",
            "Cancel, I don't want to change lock status my output",
            "Continue and change lock status",
            false, true) ) {
        return false;
    }

    lockMessageWasShown = true;
    return lockMessageWasShown;
}

void Outputs::onSgnNewNotificationMessage(int level, QString message) {
    Q_UNUSED(level)
    if (message.contains("Changing status for output")) {
        on_refreshButton_clicked();
    }
}


}  // end namespace wnd


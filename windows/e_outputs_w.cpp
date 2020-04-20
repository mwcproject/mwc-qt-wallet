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
#include "../state/e_outputs.h"
#include "../util/stringutils.h"
#include "../core/appcontext.h"
#include <QDebug>
#include "../dialogs/e_showoutputdlg.h"
#include "../state/timeoutlock.h"
#include "../core/HodlStatus.h"

namespace wnd {

Outputs::Outputs(QWidget *parent, state::Outputs *_state) :
        core::NavWnd(parent, _state->getContext()),
        ui(new Ui::Outputs),
        state(_state) {
    ui->setupUi(this);

    ui->outputsTable->setHightlightColors(QColor(255, 255, 255, 51), QColor(255, 255, 255, 153)); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->outputsTable->setStripeAlfaDelta(5); // very small number


    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    bool showAll = state->getContext()->appContext->isShowOutputAll();
    ui->showAll->setEnabled(!showAll); // inverse state, enabled is a switch
    ui->showUnspent->setEnabled(showAll);

    QString accName = updateWalletBalance();

    inHodl = state->getContext()->hodlStatus->isInHodl();

    initTableHeaders();

    requestOutputs(accName);

    ui->outputsTable->setFocus();

}

Outputs::~Outputs() {
    state->getContext()->appContext->setShowOutputAll( isShowUnspent() );
    saveTableHeaders();
    state->deleteWnd(this);
    delete ui;
}

void Outputs::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if (widths.size() != 8) {
        widths = QVector<int>{40, 90, 100, 70, 240, 50, 70, 70};
    }
    Q_ASSERT(widths.size() == 8);

    ui->outputsTable->setColumnWidths(widths);

    if (inHodl) {
        ui->outputsTable->setColumnCount(widths.size()+1);
        ui->outputsTable->setColumnWidth(widths.size(),60);
        QTableWidgetItem * itm = new QTableWidgetItem("HODL") ;
        ui->outputsTable->setHorizontalHeaderItem( widths.size(), itm );
//                horizontalHeaderItem( widths.size() )->setText( "HODL" );
    }
}

void Outputs::saveTableHeaders() {
    QVector<int>  width = ui->outputsTable->getColumnWidths();
    if (inHodl)
        width.pop_back();
    state->updateColumnsWidhts( width );
}

int Outputs::calcPageSize() const {
    QSize sz1 = ui->outputsTable->size();
    QSize sz2 = ui->progressFrame->size();

    return ListWithColumns::getNumberOfVisibleRows(std::max(sz1.height(), std::max(0, sz2.height() - 50)));
}


void Outputs::setOutputCount(QString account, int count) {
    // Init arrays and request the data...
    totalOutputs = count;

    if (account != currentSelectedAccount()) {
        qDebug() << "Outputs::setOutputCount ignored because of account name";
        return;
    }

    int pageSize = calcPageSize();
    currentPagePosition = std::max(0, totalOutputs-pageSize);
    buttonState = updatePages(currentPagePosition, totalOutputs, pageSize);
}

void Outputs::on_prevBtn_clicked() {
    if (currentPagePosition > 0) {
        int pageSize = calcPageSize();
        currentPagePosition = std::max( 0, currentPagePosition-pageSize );

        buttonState = updatePages(currentPagePosition, totalOutputs, pageSize);
        state->requestOutputs(currentSelectedAccount(), isShowUnspent(), false);

        // progress make it worse
//        ui->progressFrame->show();
//        ui->tableFrame->hide();
    }
}

void Outputs::on_nextBtn_clicked() {
    if (currentPagePosition + outputs.size() < totalOutputs) {
        int pageSize = calcPageSize();
        currentPagePosition = std::min( totalOutputs-pageSize, currentPagePosition+pageSize );

        buttonState = updatePages(currentPagePosition, totalOutputs, pageSize);

        state->requestOutputs(currentSelectedAccount(), isShowUnspent(), false);

        // progress make it worse
//        ui->progressFrame->show();
//        ui->tableFrame->hide();
    }
}

QPair<bool, bool> Outputs::updatePages(int currentPos, int total, int pageSize) {
    ui->nextBtn->setEnabled(false);
    ui->prevBtn->setEnabled(false);
    if (currentPos < 0 || total <= 0 || pageSize <= 0) {
        ui->pageLabel->setText("");
        return QPair<bool, bool>(false, false);
    } else {
        if (total <= 1) {
            ui->pageLabel->setText(QString::number(total) +
                                   " of " + QString::number(total));
        } else {
            ui->pageLabel->setText(QString::number(currentPos + 1) + "-" +
                                   QString::number(std::min(currentPos + pageSize - 1 + 1, total)) +
                                   " of " + QString::number(total));
        }
        return QPair<bool, bool>(currentPos > 0, currentPos < total - pageSize);
    }
}

QString Outputs::currentSelectedAccount() {
    int curIdx = ui->accountComboBox->currentIndex();

    if (curIdx >= 0 && curIdx < accountInfo.size())
        return accountInfo[curIdx].accountName;

    return "";
}


void Outputs::setOutputsData(QString account, int64_t height, const QVector<wallet::WalletOutput> &outp) {
    Q_UNUSED(height)
    Q_ASSERT(totalOutputs == outp.size());

    qDebug() << "Outputs::setOutputsData for account=" << account << " outp zs=" << outp.size();

    ui->progressFrame->hide();
    ui->tableFrame->show();

    if (account != currentSelectedAccount()) {
        qDebug() << "Outputs::setOutputsData ignored because of account name";
        return;
    }

    outputs.clear();
    int pageSize = calcPageSize();
    for (int i=currentPagePosition; i<outp.size() && pageSize>0; i++, pageSize--) {
        outputs.push_back(outp[i]);
    }

    ui->outputsTable->clearData();

    qDebug() << "updating output table for " << outputs.size() << " rows";
    for (int i = outputs.size()-1; i >= 0; i--) {
        auto &out = outputs[i];

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

        if (inHodl) {
            core::HodlOutputInfo hodlOut = state->getContext()->hodlStatus->getHodlOutput( out.outputCommitment );
            rowData.push_back( hodlOut.cls.isEmpty() ? "No" : hodlOut.cls );
        }

        ui->outputsTable->appendRow( rowData );
    }

    ui->prevBtn->setEnabled(buttonState.first);
    ui->nextBtn->setEnabled(buttonState.second);
}

void Outputs::triggerRefresh() {
    if (ui->progressFrame->isHidden()) {
        on_refreshButton_clicked();
    }
}


void Outputs::on_refreshButton_clicked() {
    ui->progressFrame->show();
    ui->tableFrame->hide();
    requestOutputs(currentSelectedAccount());
}


// Request and reset page counter
void Outputs::requestOutputs(QString account) {

    currentPagePosition = INT_MAX; // Reset Paging

    ui->progressFrame->show();
    ui->tableFrame->hide();

    updatePages(-1, -1, -1);

    ui->outputsTable->clearData();
    state->requestOutputs(account, isShowUnspent(), true);
}

void Outputs::on_accountComboBox_activated(int index) {
    if (index >= 0 && index < accountInfo.size()) {
        currentPagePosition = INT_MAX; // Reset Paging
        state->switchCurrentAccount(accountInfo[index]);
        requestOutputs(accountInfo[index].accountName);
    }
}

QString Outputs::updateWalletBalance() {
    accountInfo = state->getWalletBalance();
    QString selectedAccount = state->getCurrentAccountName();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx = 0;
    for (auto &info : accountInfo) {
        if (info.accountName == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem(info.getLongAccountName(), QVariant(idx++));
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
    return accountInfo[selectedAccIdx].accountName;
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
    if (row<0 || row>=outputs.size())
        return nullptr;

    return &outputs[outputs.size()-1-row];
}

void Outputs::on_outputsTable_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    state::TimeoutLockObject to( state );
    wallet::WalletOutput * selected = getSelectedOutput();

    if (selected==nullptr)
        return;

    QString account = currentSelectedAccount();
    QString outputNote = state->getContext()->appContext->getNote(account, selected->outputCommitment);
    dlg::ShowOutputDlg showOutputDlg(this, account, *selected, state->getContext()->wallet->getWalletConfig(), state->getContext()->hodlStatus, outputNote );
    connect(&showOutputDlg, &dlg::ShowOutputDlg::saveOutputNote, this, &Outputs::saveOutputNote);
    showOutputDlg.exec();
}

void Outputs::saveOutputNote(const QString& account, const QString& commitment, const QString& note) {
    if (note.isEmpty()) {
        state->getContext()->appContext->deleteNote(account, commitment);
    }
    else {
        // add new note or update existing note for this commitment
        state->getContext()->appContext->updateNote(account, commitment, note);
    }
}

}  // end namespace wnd


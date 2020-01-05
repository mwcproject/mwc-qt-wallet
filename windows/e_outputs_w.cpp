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
#include <QDebug>
#include "../core/HodlStatus.h"

namespace wnd {

Outputs::Outputs(QWidget *parent, state::Outputs * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::Outputs),
    state(_state)
{
    ui->setupUi(this);

    ui->outputsTable->setHightlightColors(QColor(255,255,255,51), QColor(255,255,255,153) ); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->outputsTable->setStripeAlfaDelta( 5 ); // very small number


    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    QString accName = updateWalletBalance();

    inHodl = state->getContext()->hodlStatus->isInHodl();

    initTableHeaders();

    requestOutputs(accName);

    ui->outputsTable->setFocus();

    updatePages(-1, -1, -1);
}

Outputs::~Outputs()
{
    saveTableHeaders();
    state->deleteWnd(this);
    delete ui;
}

void Outputs::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 7 ) {
        widths = QVector<int>{40,90,70,50,240,70,70};
    }
    Q_ASSERT( widths.size() == 7 );

    ui->outputsTable->setColumnWidths(widths);

    if (inHodl) {
        ui->outputsTable->setColumnCount(widths.size()+1);
        ui->outputsTable->setColumnWidth(widths.size(),60);
        ui->outputsTable->horizontalHeaderItem( widths.size() )->setText( "HODL" );
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

    return ListWithColumns::getNumberOfVisibleRows( std::max(sz1.height(), std::max(0, sz2.height()-50) ) );
}


void Outputs::setOutputCount(QString account, int count) {
    // Init arrays and request the data...
    totalOutputs = count;

    if ( account != currentSelectedAccount() ) {
        qDebug() << "Outputs::setOutputCount ignored because of account name";
        return;
    }

    int pageSize = calcPageSize();
    currentPagePosition = std::max(0, std::min(currentPagePosition, totalOutputs-pageSize));
    buttonState = updatePages(currentPagePosition, totalOutputs, pageSize);

    // Requesting the output data
    state->requestOutputs(account, currentPagePosition, pageSize);
}

void Outputs::on_prevBtn_clicked()
{
    if (currentPagePosition > 0) {
        int pageSize = calcPageSize();
        currentPagePosition = std::max( 0, currentPagePosition-pageSize );

        buttonState = updatePages(currentPagePosition, totalOutputs, pageSize);
        state->requestOutputs(currentSelectedAccount(), currentPagePosition, pageSize);

        ui->progressFrame->show();
        ui->tableFrame->hide();
    }
}

void Outputs::on_nextBtn_clicked()
{
    if (currentPagePosition + outputs.size() < totalOutputs ) {
        int pageSize = calcPageSize();
        currentPagePosition = std::min( totalOutputs-pageSize, currentPagePosition+pageSize );

        buttonState = updatePages(currentPagePosition, totalOutputs, pageSize);
        state->requestOutputs(currentSelectedAccount(), currentPagePosition, pageSize);

        ui->progressFrame->show();
        ui->tableFrame->hide();
    }
}

QPair<bool,bool> Outputs::updatePages( int currentPos, int total, int pageSize ) {
    ui->nextBtn->setEnabled(false);
    ui->prevBtn->setEnabled(false);
    if (currentPos <0 || total<=0 || pageSize<=0) {
        ui->pageLabel->setText("");
        return QPair<bool,bool>(false,false);
    }
    else {
        if (total <= 1) {
            ui->pageLabel->setText( QString::number(total) +
                                    " of " + QString::number( total) );
        }
        else {
            ui->pageLabel->setText( QString::number(currentPos+1) + "-" + QString::number( std::min(currentPos+pageSize-1+1, total) ) +
                                    " of " + QString::number( total) );
        }
        return QPair<bool,bool>(currentPos>0,currentPos < total-pageSize);
    }
}

QString Outputs::currentSelectedAccount() {
    int curIdx = ui->accountComboBox->currentIndex();

    if ( curIdx>=0 && curIdx<accountInfo.size() )
        return accountInfo[curIdx].accountName;

    return "";
}


void Outputs::setOutputsData(QString account, int64_t height, const QVector<wallet::WalletOutput> & outp ) {
    Q_UNUSED(height)

    qDebug() << "Outputs::setOutputsData for account=" << account << " outp zs=" << outp.size();

    ui->progressFrame->hide();
    ui->tableFrame->show();


    if ( account != currentSelectedAccount() ) {
        qDebug() << "Outputs::setOutputsData ignored because of account name";
        return;
    }

    outputs = outp;

    int rowNum = outputs.size();

    ui->outputsTable->clearData();

    qDebug() << "updating output table for " << rowNum << " rows";
    for ( int i=0; i<rowNum; i++ ) {
        auto & out = outputs[i];

        QVector<QString> rowData{
            QString::number( out.txIdx+1 ),
           // out.status, // Status allways 'unspent', so no reasons to print it.
            util::nano2one(out.valueNano),
            out.numOfConfirms,
            out.coinbase ? "Yes":"No",
            out.outputCommitment,
            out.MMRIndex,
            out.lockedUntil
        };

        if (inHodl) {
            rowData.push_back( state->getContext()->hodlStatus->isOutputInHODL( out.outputCommitment ) ? "Yes" : "No" );
        }

        ui->outputsTable->appendRow( rowData );
    }

    ui->prevBtn->setEnabled( buttonState.first );
    ui->nextBtn->setEnabled( buttonState.second );
}

void wnd::Outputs::on_refreshButton_clicked()
{
    ui->progressFrame->show();
    ui->tableFrame->hide();
    // Request count and then refresh from current posiotion...
    state->requestOutputCount( currentSelectedAccount() );
    // count will trigger the page update
}


// Request and reset page counter
void Outputs::requestOutputs(QString account) {

    currentPagePosition = INT_MAX; // Reset Paging

    ui->progressFrame->show();
    ui->tableFrame->hide();

    updatePages(-1, -1, -1);

    ui->outputsTable->clearData();
    state->requestOutputCount(account);
}

void Outputs::on_accountComboBox_activated(int index)
{
    if (index>=0 && index<accountInfo.size()) {
        currentPagePosition = INT_MAX; // Reset Paging
        state->switchCurrentAccount( accountInfo[index] );
        requestOutputs( accountInfo[index].accountName );
    }
}

QString Outputs::updateWalletBalance() {
    accountInfo = state->getWalletBalance();
    QString selectedAccount = state->getCurrentAccountName();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx=0;
    for (auto & info : accountInfo) {
        if (info.accountName == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( info.getLongAccountName(), QVariant(idx++) );
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
    return accountInfo[selectedAccIdx].accountName;
}

}


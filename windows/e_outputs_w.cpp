#include "e_outputs_w.h"
#include "ui_e_outputs.h"
#include "../state/e_outputs.h"
#include "../util/stringutils.h"
#include <QDebug>

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
        widths = QVector<int>{70,80,70,70,200,70,70};
    }
    Q_ASSERT( widths.size() == 7 );

    ui->outputsTable->setColumnWidths(widths);
}

void Outputs::saveTableHeaders() {
    state->updateColumnsWidhts( ui->outputsTable->getColumnWidths() );
}

int Outputs::calcPageSize() const {
    QSize sz1 = ui->outputsTable->size();
    QSize sz2 = ui->progressFrame->size();

    return ListWithColumns::getNumberOfVisibleRows( std::max(sz1.height(), std::max(0, sz2.height()-50) ) );
}


void Outputs::setOutputCount(QString account, int count) {
    // Init arrays and request the data...
    currentPagePosition = 0; // position at the paging...
    totalOutputs = count;

    if ( account != currentSelectedAccount() ) {
        qDebug() << "Outputs::setOutputCount ignored because of account name";
        return;
    }

    int pageSize = calcPageSize();
    currentPagePosition = std::max(0, totalOutputs-pageSize);
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

    qDebug() << "Outputs::setOutputsData for account=" << account << " height=" << height << " outp zs=" << outp.size();

    ui->progressFrame->hide();
    ui->tableFrame->show();

    Q_UNUSED(height);

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

        ui->outputsTable->appendRow( QVector<QString>{
                                         out.txIdx+1,
                                        // out.status, // Status allways 'unspent', so no reasons to print it.
                                         util::nano2one(out.valueNano),
                                         out.numOfConfirms,
                                         out.coinbase ? "Yes":"No",
                                         out.outputCommitment,
                                         out.MMRIndex,
                                         out.lockedUntil
                                     } );
    }

    ui->prevBtn->setEnabled( buttonState.first );
    ui->nextBtn->setEnabled( buttonState.second );
}

// Request and reset page counter
void Outputs::requestOutputs(QString account) {

    ui->progressFrame->show();
    ui->tableFrame->hide();

    updatePages(-1, -1, -1);

    ui->outputsTable->clearData();
    state->requestOutputCount(account);
}

void Outputs::on_accountComboBox_activated(int index)
{
    if (index>=0 && index<accountInfo.size()) {
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


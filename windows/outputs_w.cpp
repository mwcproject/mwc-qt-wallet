#include "outputs_w.h"
#include "ui_outputs.h"
#include "../state/outputs.h"
#include "../util/stringutils.h"

namespace wnd {

Outputs::Outputs(QWidget *parent, state::Outputs * _state) :
    QWidget(parent),
    ui(new Ui::Outputs),
    state(_state)
{
    ui->setupUi(this);

    ui->outputGroupBox->setTitle("Outputs for account " + state->getCurrentAccountName());

    initTableHeaders();
    updateOutputsTable();
}

Outputs::~Outputs()
{
    saveTableHeaders();
    delete ui;
}

void Outputs::initTableHeaders() {

    // Disabling to show the grid
    ui->outputsTable->setShowGrid(false);

    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 10 ) {
        widths = QVector<int>{10,10,30,30,30,30,100,30,30,30};
    }
    Q_ASSERT( widths.size() == 10 );
    Q_ASSERT( ui->outputsTable->columnCount() == widths.size() );

    for (int u=0;u<widths.size();u++)
        ui->outputsTable->setColumnWidth(u,widths[u]);
}

void Outputs::saveTableHeaders() {
    int cc = ui->outputsTable->columnCount();
    QVector<int> widths(cc);
    for (int t=0;t<cc;t++)
        widths[t] = ui->outputsTable->columnWidth(t);

    state->updateColumnsWidhts(widths);
}

void Outputs::updateOutputsTable() {

    QVector<wallet::WalletOutput> outputs = state->getOutputs();
    int rowNum = outputs.size();

    QTableWidget * tt = ui->outputsTable;

    tt->clearContents();
    tt->setRowCount(rowNum);

    Q_ASSERT( ui->outputsTable->columnCount() == 10 );
    tt->setSortingEnabled(false);

    for ( int i=0; i<rowNum; i++ ) {
        auto & out = outputs[i];

        tt->setItem( i, 0, new QTableWidgetItem(QString::number(i+1) ));
        tt->setItem( i, 1, new QTableWidgetItem( QString::number(out.txIdx) ) );
        tt->setItem( i, 2, new QTableWidgetItem( out.getStatusStr() ) );
        tt->setItem( i, 3, new QTableWidgetItem( util::nano2one(out.valueNano) ) );
        tt->setItem( i, 4, new QTableWidgetItem( QString::number( out.numOfConfirms) ) );
        tt->setItem( i, 5, new QTableWidgetItem( out.coinbase ? "Yes":"No" ) );
        tt->setItem( i, 6, new QTableWidgetItem( out.outputCommitment ) );
        tt->setItem( i, 7, new QTableWidgetItem( QString::number( out.MMRIndex) ) );
        tt->setItem( i, 8, new QTableWidgetItem( QString::number( out.lockHeight )));
        tt->setItem( i, 9, new QTableWidgetItem( QString::number( out.lockedUntil )));
    }
}


}

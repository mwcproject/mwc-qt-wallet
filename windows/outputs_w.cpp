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


    QString title = "Outputs for account " + state->getCurrentAccountName();
    state->setWindowTitle(title);
    ui->outputGroupBox->setTitle(title);

    initTableHeaders();
    updateOutputsTable();

    ui->outputsTable->setFocus();
}

Outputs::~Outputs()
{
    saveTableHeaders();
    delete ui;
}

void Outputs::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 9 ) {
        widths = QVector<int>{30,90,50,60,50,200,70,90,90};
    }
    Q_ASSERT( widths.size() == 9 );

    ui->outputsTable->setColumnWidths(widths);
}

void Outputs::saveTableHeaders() {
    state->updateColumnsWidhts( ui->outputsTable->getColumnWidths() );
}

void Outputs::updateOutputsTable() {

    QVector<wallet::WalletOutput> outputs = state->getOutputs();
    int rowNum = outputs.size();

    ui->outputsTable->clearData();

    for ( int i=0; i<rowNum; i++ ) {
        auto & out = outputs[i];

        ui->outputsTable->appendRow( QVector<QString>{
                                         QString::number(out.txIdx),
                                         out.getStatusStr(),
                                         util::nano2one(out.valueNano),
                                         QString::number( out.numOfConfirms),
                                         out.coinbase ? "Yes":"No",
                                         out.outputCommitment,
                                         QString::number( out.MMRIndex),
                                         QString::number( out.lockHeight ),
                                         QString::number( out.lockedUntil )
                                     }, ListWithColumns::SELECTION::NORMAL );
    }
}


}

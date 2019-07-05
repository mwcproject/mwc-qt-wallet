#include "e_outputs_w.h"
#include "ui_e_outputs.h"
#include "../state/e_outputs.h"
#include "../util/stringutils.h"

namespace wnd {

Outputs::Outputs(QWidget *parent, state::Outputs * _state) :
    core::NavWnd(parent, _state->getStateMachine(), _state->getAppContext() ),
    ui(new Ui::Outputs),
    state(_state)
{
    ui->setupUi(this);

    ui->outputsTable->setHightlightColors(QColor(255,255,255,51), QColor(255,255,255,153) ); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->outputsTable->setStripeAlfaDelta( 5 ); // very small number


    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    updateWalletBalance();

    initTableHeaders();

    requestOutputs();

    ui->outputsTable->setFocus();
}

Outputs::~Outputs()
{
    saveTableHeaders();
    state->deleteWnd();
    delete ui;
}

void Outputs::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 8 ) {
        widths = QVector<int>{70,100,80,70,70,200,70,70};
    }
    Q_ASSERT( widths.size() == 8 );

    ui->outputsTable->setColumnWidths(widths);
}

void Outputs::saveTableHeaders() {
    state->updateColumnsWidhts( ui->outputsTable->getColumnWidths() );
}

void Outputs::setOutputsData(QString account, int64_t height, const QVector<wallet::WalletOutput> & outp ) {

    ui->progressFrame->hide();
    ui->outputsTable->show();

    Q_UNUSED(height);

    // Update active account just to guarantee be in sync. Normally account should be the same
    int curIdx = ui->accountComboBox->currentIndex();

    bool accountOK = false;
    if ( curIdx>=0 && curIdx<accountInfo.size() ) {
        accountOK = (accountInfo[curIdx].accountName == account);
    }

    if (!accountOK) {
        for ( int t=0; t<accountInfo.size(); t++ ) {
            if (accountInfo[t].accountName == account) {
                ui->accountComboBox->setCurrentIndex(t);
                accountOK = true;
                break;
            }
        }
    }

    Q_ASSERT(accountOK);

    outputs = outp;

    int rowNum = outputs.size();

    ui->outputsTable->clearData();

    for ( int i=0; i<rowNum; i++ ) {
        auto & out = outputs[i];

        ui->outputsTable->appendRow( QVector<QString>{
                                         out.txIdx,
                                         out.status,
                                         util::nano2one(out.valueNano),
                                         out.numOfConfirms,
                                         out.coinbase ? "Yes":"No",
                                         out.outputCommitment,
                                         out.MMRIndex,
                                         out.lockedUntil
                                     } );
    }
}

void Outputs::requestOutputs() {

    ui->progressFrame->show();
    ui->outputsTable->hide();

    ui->outputsTable->clearData();
    state->requestOutputs();
}

void Outputs::on_accountComboBox_activated(int index)
{
    if (index>=0 && index<accountInfo.size()) {
        state->switchCurrentAccount( accountInfo[index] );
        requestOutputs();
    }
}

void Outputs::updateWalletBalance() {
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
}

}


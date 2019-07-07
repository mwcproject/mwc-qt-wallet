#include "airdrop_w.h"
#include "ui_airdrop.h"
#include "../state/airdrop.h"
#include "../control/messagebox.h"
#include "../util/stringutils.h"

namespace wnd {

Airdrop::Airdrop(QWidget *parent, state::Airdrop * _state) :
    QWidget(parent),
    ui(new Ui::Airdrop),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Claim MWC for your Airdrop account");

    initTableHeaders();
    updateClaimStatus();

    ui->btcAddressEdit->setFocus();
}

Airdrop::~Airdrop()
{
    saveTableHeaders();
    delete ui;
}

void Airdrop::on_nextButton_clicked()
{
    QString address = ui->btcAddressEdit->text().trimmed();

    if (address.length()==0) {
        control::MessageBox::message(this, "Error", "Please input your BTC address that you registered at airdrop");
        return;
    }

    QPair<bool, QString> res = state->claimBtcAddress(address);
    if (!res.first) {
        control::MessageBox::message(this, "Claim failure", "You claim for MWC was failed.\nError: " + res.second );
        return;
    }
}

void wnd::Airdrop::on_refreshClaimsButton_clicked()
{
    updateClaimStatus();
}

void Airdrop::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 4 ) {
        widths = QVector<int>{30,250,100,50};
    }
    Q_ASSERT( widths.size() == 4 );
    ui->claimsTable->setColumnWidths( widths );
}

void Airdrop::saveTableHeaders() {
    state->updateColumnsWidhts( ui->claimsTable->getColumnWidths() );
}

void Airdrop::updateClaimStatus() {
    QTableWidget * tt = ui->claimsTable;
    tt->clearContents();

    QVector<state::AirdropRequestsStatus> claimStatus = state->getAirdropStatusInfo();

    tt->setRowCount(claimStatus.size());

    Q_ASSERT( tt->columnCount() == 4 );
    tt->setSortingEnabled(false);

    for ( int i=0; i<claimStatus.size(); i++ ) {
        auto & status = claimStatus[i];

        tt->setItem( i, 0, new QTableWidgetItem(QString::number(i+1) ));
        tt->setItem( i, 1, new QTableWidgetItem( status.btcAddress ));
        tt->setItem( i, 2, new QTableWidgetItem( status.status ));
        tt->setItem( i, 3, new QTableWidgetItem( util::nano2one(status.nanoCoins) ));
    }

}


}



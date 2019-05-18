#include "airdrop_w.h"
#include "ui_airdrop.h"
#include "../state/airdrop.h"
#include <QMessageBox>
#include "../util/stringutils.h"

namespace wnd {

Airdrop::Airdrop(QWidget *parent, state::Airdrop * _state) :
    QWidget(parent),
    ui(new Ui::Airdrop),
    state(_state)
{
    ui->setupUi(this);

    updateClaimStatus();
}

Airdrop::~Airdrop()
{
    delete ui;
}

void Airdrop::on_nextButton_clicked()
{
    QString address = ui->btcAddressEdit->text();

    if (address.length()==0) {
        QMessageBox::critical(this, "MWC claim", "Please input your BTC address that you registered at airdrop");
        return;
    }

    QPair<bool, QString> res = state->claimBtcAddress(address);
    if (!res.first) {
        QMessageBox::critical(this, "MWC claim", "You claim for MWC was failed. Error: " + res.second );
        return;
    }
}

void wnd::Airdrop::on_refreshClaimsButton_clicked()
{
    updateClaimStatus();
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



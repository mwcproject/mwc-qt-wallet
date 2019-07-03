#include "hodl_w.h"
#include "ui_hodl.h"
#include "../state/hodl.h"
#include "../control/messagebox.h"
#include <QAbstractItemView>

namespace wnd {

Hodl::Hodl(QWidget *parent, state::Hodl * _state) :
    QWidget(parent),
    ui(new Ui::Hodl),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("HODL");

    ui->transactionsListWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );

    // fill transaction list with transactions
    transactions = state->getTransactions();

    int idx = -1;
    for ( auto & trans : transactions  ) {
        idx++;

        // skipping all negative and non confirmed
        if (trans.coinNano<=0 || !trans.confirmed)
            continue;

        QListWidgetItem * itm = new QListWidgetItem(trans.toStringShort(), ui->transactionsListWidget);
        itm->setData( Qt::UserRole, QVariant(idx) );
        ui->transactionsListWidget->addItem(itm);
    }
}

Hodl::~Hodl()
{
    delete ui;
}

void Hodl::on_submitAllTransactionsButton_clicked()
{
    control::MessageBox::RETURN_CODE res = control::MessageBox::question(this, "Warning",
                         "You are submitting all your MWC to HODL program. Those funds will be moved to your new account. This process might take some time. Please press 'Process' to start processing", "Process", "Cancel", false, true);
    if (res!=control::MessageBox::BTN1)
        return;

    // Check what is submitted
    QVector<QString> submittedTransactions;
    for (auto & trans : transactions)
        submittedTransactions.push_back( trans.txid );

    state->submitForHodl(submittedTransactions);
}

void Hodl::on_submitSelectedTransactionsButton_clicked()
{
    QList<QListWidgetItem *> selectedItems = ui->transactionsListWidget->selectedItems();
    if (selectedItems.size()==0) {
        control::MessageBox::message(this, "Need info",
                             "Please select transactions that you want to submit to HODL program.");
        return;
    }

    // Check what is submitted
    QVector<QString> submittedTransactions;

    int64_t totalAmount = 0;
    for (QListWidgetItem * selItm : selectedItems) {
        int idx = selItm->data(Qt::UserRole).toInt();
        submittedTransactions.push_back( transactions[idx].txid );
        totalAmount += transactions[idx].coinNano;
    }

    control::MessageBox::RETURN_CODE res = control::MessageBox::question(this, "Transactions",
                         "You are submitting " + util::nano2one(totalAmount)  + " MWC to HODL program. Those funds will be moved to your new account. This process might take some time. Please press 'Process' to start processing",
                                             "Process", "Cancel", false, true);

    if (res == control::MessageBox::BTN1) {
        state->submitForHodl(submittedTransactions);
    }
}


}


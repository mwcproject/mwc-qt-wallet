#include "sendcoins_w.h"
#include "ui_sendcoins.h"
#include "../state/sendcoins.h"
#include "selectcontact.h"
#include <QMessageBox>
#include "sendcoinsparamsdialog.h"

namespace wnd {

SendCoins::SendCoins(QWidget *parent, state::SendCoins * _state) :
    QWidget(parent),
    ui(new Ui::SendCoins),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Send MWC");

    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 6 ) {
        widths = QVector<int>{30,60,60,300,60,130};
    }
    Q_ASSERT( widths.size() == 6 );

    ui->transactionTable->setColumnWidths(widths);
    updateTransactionList();

    ui->sendEdit->setFocus();
}

SendCoins::~SendCoins()
{
    state->updateColumnsWidhts( ui->transactionTable->getColumnWidths() );

    delete ui;
}


void SendCoins::on_fromContactButton_clicked()
{
    QVector<wallet::WalletContact> contacts = state->getContacts();

    SelectContact contDlg(this, contacts);
    if ( contDlg.exec() == QDialog::Accepted) {
        wallet::WalletContact contact = contDlg.getSelectedContact();
        if (contact.name.length()>0) {
            ui->sendToAddress->setText( contact.address );
        }
    }
}

void SendCoins::updateTransactionList() {
    QVector<wallet::WalletTransaction> transactions = state->getTransactions(10);

    ui->transactionTable->clearData();

    for ( auto ti=transactions.rbegin(); ti!=transactions.rend(); ti++ ) {

        ui->transactionTable->appendRow(QVector<QString>{
                    QString::number( ti->txIdx ),
                    ti->getTypeAsStr(),
                    util::nano2one(ti->coinNano),
                    ti->address,
                    ti->confirmed ? "Yes":"No",
                    ti->confirmationTime
                    });
    }
}

void SendCoins::on_sendButton_clicked()
{
    QString coinStr = ui->sendEdit->text();

    // Parse number of coins
    bool ok = false;
    double coinDbl = coinStr.toDouble(&ok);

    if (!ok) {
        QMessageBox::critical(this, "Parsing error", "Unable to parse the number of coins to send");
        return;
    }

    if (coinDbl<=0.0) {
        QMessageBox::critical(this, "Parsing error", "Please input correct positive number of MWC to send");
        return;
    }
    // to nanos
    long coinsNano = long((coinDbl * 1000000000.0) + 0.5);

    // Address to send MWC
    QString address = ui->sendToAddress->text();
    if ( address.length() == 0 ) {
        QMessageBox::critical(this, "Parsing error", "Please define address where to send MWC");
        return;
    }

    // Message to include into transaction
    QString message = ui->messageEdit->text();


    QPair<bool, QString> res = state->sendCoins( coinsNano,  address,  message );

    updateTransactionList();

    if (res.first) {
        QMessageBox::information(this, "Send MWC coins", "You was able sucessfully send " + coinStr + " MWC to address " + address);
        // Let's clean up all fields
        ui->sendEdit->clear();
        ui->sendToAddress->clear();
        ui->messageEdit->clear();
        return;
    }
    else {
        QMessageBox::critical(this, "Send MWC coins", "Wallet wasn't be able to send " + coinStr + " MWC to address " + address + " due error: " + res.second);
        return;
    }

}

void wnd::SendCoins::on_settingsButton_clicked()
{
    state::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }
}

}

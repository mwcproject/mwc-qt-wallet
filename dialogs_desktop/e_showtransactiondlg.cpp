// Copyright 2020 The MWC Developers
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

#include "e_showtransactiondlg.h"
#include "ui_e_showtransactiondlg.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"

namespace dlg {

ShowTransactionDlg::ShowTransactionDlg(QWidget *parent,
                                       const QString& account,
                                       const wallet::WalletTransaction transaction,
                                       const QVector<wallet::WalletOutput> & _outputs,
                                       const QVector<QString> & messages,
                                       const QString& note) :
    control::MwcDialog(parent),
    ui(new Ui::ShowTransactionDlg),
    outputs(_outputs)
{
    ui->setupUi(this);
    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    util = new bridge::Util(this);

    blockExplorerUrl = config->getBlockExplorerUrl(config->getNetwork());

    ui->titleLabel->setText("Transaction #" + QString::number(transaction.txIdx+1) ); // Need to show 1 base indexes, instead of 0 based.
    ui->type->setText( transaction.getTypeAsStr() );
    ui->id->setText( transaction.txid );
    ui->address->setText(transaction.address);
    ui->mwc->setText( util::nano2one( transaction.coinNano ) );
    ui->confirm->setText( transaction.confirmed ? "Yes" : "No" );

    QString strMessage = "None";
    if (!messages.isEmpty()) {
        strMessage = messages[0];
        for (int t=1;t<messages.size();t++) {
            strMessage += "; ";
            strMessage += messages[t];
        }
    }

    ui->messages->setText(strMessage);

    ui->kernel->setText(transaction.kernel);
    ui->numInputs->setText( QString::number(transaction.numInputs) );
    ui->numOutputs->setText( QString::number(transaction.numOutputs) );
    ui->credited->setText( util::nano2one( transaction.credited ) );
    ui->debited->setText( util::nano2one( transaction.debited ) );
    ui->fee->setText( util::nano2one( transaction.fee ) );

    ui->commitsComboBox->clear();
    for (int i=0; i<outputs.size(); i++) {
        QString commitType;
        if (i<transaction.numInputs) {
            commitType = "Input " + QString::number(i+1) + ": ";
        }
        else {
            commitType = "Output " + QString::number(i-transaction.numInputs+1) + ": ";
        }

        ui->commitsComboBox->addItem( commitType + outputs[i].outputCommitment, QVariant(i));
    }

    // Selecting first output
    if (transaction.numInputs < outputs.size()) {
        ui->commitsComboBox->setCurrentIndex(transaction.numInputs);
        updateOutputData();
    }
    else {
        ui->commitsComboBox->setCurrentIndex(-1);
    }

    txUuid = transaction.txid;
    this->account = account;
    newTransactionNote = note;
    ui->transactionNote->setText(newTransactionNote);
}

ShowTransactionDlg::~ShowTransactionDlg()
{
    delete ui;
}

void ShowTransactionDlg::updateOutputData() {
    auto dt = ui->commitsComboBox->currentData();
    if (!dt.isValid()) {
        // Need to clean uo the data
        ui->out_status->setText("");
        ui->out_mwc->setText("");
        ui->out_height->setText( "" );
        ui->out_confirms->setText( "" );
        ui->out_coinBase->setText("");
        ui->out_tx->setText("");
        ui->out_label1->hide();
        ui->out_label2->hide();
        ui->out_label3->hide();
        ui->out_label4->hide();
        ui->out_label5->hide();
        ui->out_label6->hide();
        return;
    }

    wallet::WalletOutput & out = outputs[ dt.toInt() ];
    ui->out_label1->show();
    ui->out_label2->show();
    ui->out_label3->show();
    ui->out_label4->show();
    ui->out_label5->show();
    ui->out_label6->show();
    ui->out_status->setText(out.status);
    ui->out_mwc->setText(util::nano2one( out.valueNano) );
    ui->out_height->setText( out.blockHeight );
    ui->out_confirms->setText( out.numOfConfirms );
    ui->out_coinBase->setText(out.coinbase?"Yes":"No");
    ui->out_tx->setText(out.txIdx<0 ? "None" : QString::number(out.txIdx+1) );
}

void ShowTransactionDlg::on_viewKernel_clicked()
{
    util->openUrlInBrowser("https://" + blockExplorerUrl + "/#k" + ui->kernel->text() );
}

void ShowTransactionDlg::on_viewCommit_clicked()
{
    auto dt = ui->commitsComboBox->currentData();
    if (!dt.isValid())
        return;

    util->openUrlInBrowser("https://" + blockExplorerUrl + "/#o" + outputs[ dt.toInt() ].outputCommitment );
}

void ShowTransactionDlg::on_commitsComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    updateOutputData();
}

void ShowTransactionDlg::on_okButton_clicked()
{
    newTransactionNote = ui->transactionNote->text();
    accept();
}

void ShowTransactionDlg::on_cancelButton_clicked()
{
    reject();
}

}


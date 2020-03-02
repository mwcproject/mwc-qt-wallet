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

#include "dialogs/e_showtransactiondlg.h"
#include "ui_e_showtransactiondlg.h"
#include "../wallet/wallet.h"
#include "../util/stringutils.h"

namespace dlg {

ShowTransactionDlg::ShowTransactionDlg(QWidget *parent,  const wallet::WalletTransaction & transaction) :
    control::MwcDialog(parent),
    ui(new Ui::ShowTransactionDlg)
{
    ui->setupUi(this);

    ui->titleLabel->setText("Transaction #" + QString::number(transaction.txIdx) );
    ui->type->setText( transaction.getTypeAsStr() );
    ui->id->setText( transaction.txid );
    ui->address->setText(transaction.address);
    ui->time->setText(transaction.confirmationTime);
    ui->mwc->setText( util::nano2one( transaction.coinNano ) );
    ui->confirm->setText( transaction.confirmed ? "Yes" : "No" );
}

ShowTransactionDlg::~ShowTransactionDlg()
{
    delete ui;
}

void ShowTransactionDlg::on_pushButton_clicked()
{
    accept();
}

}

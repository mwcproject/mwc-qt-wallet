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

#ifndef SHOWTRANSACTIONDLG_H
#define SHOWTRANSACTIONDLG_H

#include "../control/mwcdialog.h"
#include "../wallet/wallet.h"

namespace Ui {
class ShowTransactionDlg;
}

namespace dlg {

class ShowTransactionDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit ShowTransactionDlg(QWidget *parent,
            const wallet::WalletTransaction transaction,
            const QVector<wallet::WalletOutput> & outputs,
            const QVector<QString> & messages);

    ~ShowTransactionDlg();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ShowTransactionDlg *ui;
};

}

#endif // SHOWTRANSACTIONDLG_H

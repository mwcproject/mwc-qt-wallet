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

#include "../control_desktop/mwcdialog.h"
#include "../wallet/wallet.h"

namespace Ui {
class ShowTransactionDlg;
}

namespace bridge {
class Wallet;
class Config;
class Util;
}

namespace dlg {

class ShowTransactionDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit ShowTransactionDlg(QWidget *parent,
            const QString& account,
            const wallet::WalletTransaction transaction,
            const QVector<wallet::WalletOutput> & outputs,
            const QVector<QString> & messages,
            const QString& note);

    ~ShowTransactionDlg();

    QString getTransactionNote() const {return newTransactionNote;}

private:
    void updateOutputData();

private slots:
    void on_okButton_clicked();
    void on_viewKernel_clicked();
    void on_viewCommit_clicked();
    void on_commitsComboBox_currentIndexChanged(int index);
    void on_cancelButton_clicked();

private:
    Ui::ShowTransactionDlg *ui;
    bridge::Wallet * wallet = nullptr;
    bridge::Config * config = nullptr;
    bridge::Util * util = nullptr;

    QVector<wallet::WalletOutput> outputs;
    QString blockExplorerUrl;

    QString account;
    QString txUuid;
    QString newTransactionNote;
};

}

#endif // SHOWTRANSACTIONDLG_H

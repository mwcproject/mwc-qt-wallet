// Copyright 2019 The MWC Developers
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

#ifndef TRANSACTIONSW_H
#define TRANSACTIONSW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Transactions;
}

namespace state {
    class Transactions;
}

namespace wnd {

class Transactions : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Transactions(QWidget *parent, state::Transactions * state);
    ~Transactions();

    void setTransactionCount(QString account, int number);
    void setTransactionData(QString account, int64_t height, const QVector<wallet::WalletTransaction> & transactions);

    void showExportProofResults(bool success, QString fn, QString msg );
    void showVerifyProofResults(bool success, QString fn, QString msg );

    void updateCancelTransacton(bool success, int64_t trIdx, QString errMessage);
    QString updateWalletBalance();


private slots:
    void on_transactionTable_itemSelectionChanged();
    void on_transactionTable_cellDoubleClicked(int row, int column);

    void on_accountComboBox_activated(int index);

    void on_refreshButton_clicked();
    void on_validateProofButton_clicked();
    void on_generateProofButton_clicked();
    void on_deleteButton_clicked();
    void on_prevBtn_clicked();
    void on_nextBtn_clicked();


private:
    // return null if nothing was selected
    wallet::WalletTransaction * getSelectedTransaction();

    void requestTransactions(QString account);

    void updateButtons();

    void initTableHeaders();
    void saveTableHeaders();

    wallet::AccountInfo getSelectedAccount() const;

    // return enable state for the buttons
    QPair<bool,bool> updatePages( int currentPos, int total, int pageSize );

    QString currentSelectedAccount();

    int calcPageSize() const;

    virtual void timerEvent(QTimerEvent *event) override;
private:
    Ui::Transactions *ui;
    state::Transactions * state;
    QVector<wallet::AccountInfo> accountInfo;
    QVector<wallet::WalletTransaction> transactions;

    int currentPagePosition = 0; // position at the paging...
    int totalTransactions = 0;

    QPair<bool,bool> buttonState = QPair<bool,bool>(false, false);
};

}

#endif // TRANSACTIONS_H

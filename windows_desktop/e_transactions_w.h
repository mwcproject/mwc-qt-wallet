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

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Transactions;
}

namespace bridge {
class Config;
class Wallet;
class Transactions;
}

namespace wnd {

class Transactions : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Transactions(QWidget *parent);
    ~Transactions();

private slots:
    void on_transactionTable_itemSelectionChanged();
    void on_transactionTable_cellDoubleClicked(int row, int column);

    void on_accountComboBox_activated(int index);

    void on_refreshButton_clicked();
    void on_validateProofButton_clicked();
    void on_generateProofButton_clicked();
    void on_exportButton_clicked();
    void on_deleteButton_clicked();
    void on_prevBtn_clicked();
    void on_nextBtn_clicked();

    void onSgnWalletBalanceUpdated();
    void onSgnTransactions( QString account, QString height, QVector<QString> transactions);
    void onSgnCancelTransacton(bool success, QString trIdx, QString errMessage);

    void onSgnTransactionById(bool success, QString account, QString height, QString transaction,
                              QVector<QString> outputs, QVector<QString> messages);

    void onSgnExportProofResult(bool success, QString fn, QString msg );
    void onSgnVerifyProofResult(bool success, QString fn, QString msg );

    void onSgnNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, QString totalDifficulty, int connections );
    void onSgnNewNotificationMessage(int level, QString message); // level: notify::MESSAGE_LEVEL values

    void saveTransactionNote(QString txUuid, QString note);
private:
    // return null if nothing was selected
    wallet::WalletTransaction * getSelectedTransaction();

    void requestTransactions();
    void updateButtons();
    void updateData();

    void initTableHeaders();
    void saveTableHeaders();

    int calcPageSize() const;
private:
    Ui::Transactions *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Transactions * transaction = nullptr; // just a placeholder to signal that this window is online

    QString account;
    QVector<wallet::WalletTransaction> allTrans;
    QVector<wallet::WalletTransaction> shownTrans;

    int currentPagePosition = 0; // position at the paging...
    int64_t nodeHeight    = 0;
};

}

#endif // TRANSACTIONS_H

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

#include <QVBoxLayout>

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"
#include "../control_desktop/richbutton.h"
#include "../control_desktop/richitem.h"
#include "../control_desktop/MwcWidget.h"

namespace Ui {
class Transactions;
}

namespace bridge {
class Config;
class Wallet;
class Transactions;
class Util;
}

namespace control {
class RichButton;
}

class QLabel;

namespace wnd {

class TransactionRecord : public control::MwcWidget {
public:
    // use 135 for unknown intialHeight value
    explicit TransactionRecord(QString id, control::RichVBox *parent, const wallet::WalletTransaction & trans,
            int expectedConfirmNumber, const QString & currentAccountPath, control::RichButtonPressCallback * btnCallBack,
            bridge::Config * config, bridge::Wallet * wallet,
            int intialHeight );

    void updateNote(const QString & note);

    int getCurrentHeight() {return size().height();}
private:
    virtual void paintEvent(QPaintEvent *) override;
private:
    // build the control instance
    control::RichItem * buildRecordContent();

private:
    QString id;
    control::RichVBox * parent;
    wallet::WalletTransaction trans;
    int expectedConfirmNumber = -1;
    QString currentAccountPath;
    control::RichButtonPressCallback * btnCallBack = nullptr;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;

    QVBoxLayout *    placeholderLayout = nullptr;
    control::RichItem * control = nullptr;

    QLabel * noteL = nullptr;
};

struct TransactionData {
    // data
    wallet::WalletTransaction trans;
    // note
    QString tx_note;
    // Lazy init record
    TransactionRecord * record = nullptr;

};

class Transactions : public core::NavWnd,  control::RichButtonPressCallback
{
    Q_OBJECT

public:
    explicit Transactions(QWidget *parent);
    ~Transactions();

private slots:
    void on_accountComboBox_activated(int index);

    void on_validateProofButton_clicked();
    void on_exportButton_clicked();

    void onSgnWalletBalanceUpdated();
    void onItemActivated(QString itemId);

protected:
    virtual void richButtonPressed(control::RichButton * button, QString coockie) override;

private:
    void requestTransactions(bool resetScroller);
    void updateData(bool resetScroller, const QVector<int> & heights);

    QVector<int> getItemsHeights();
private:
    Ui::Transactions *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Transactions * transaction = nullptr; // just a placeholder to signal that this window is online
    bridge::Util * util = nullptr;

    QVector<TransactionData> allTrans;

    bool showIntegrityAccount = false;
};

}

#endif // TRANSACTIONS_H

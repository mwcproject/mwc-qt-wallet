// Copyright 2021 The MWC Developers
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

#include "s_mktinttx_w.h"
#include "ui_s_mktinttx_w.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/swapmkt_b.h"
#include "../core/global.h"

namespace wnd {

IntegrityTransactions::IntegrityTransactions(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::IntegrityTransactions) {
    ui->setupUi(this);

    config = new bridge::Config(this);
    wallet = new bridge::Wallet(this);
    swapMarketplace = new bridge::SwapMarketplace(this);
    util = new bridge::Util(this);

    QObject::connect( wallet, &bridge::Wallet::sgnTransactions,
                      this, &IntegrityTransactions::onSgnTransactions, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnNodeStatus,
                      this, &IntegrityTransactions::onSgnNodeStatus, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnNewNotificationMessage,
                      this, &IntegrityTransactions::onSgnNewNotificationMessage, Qt::QueuedConnection);

    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    requestTransactions();

    updateData();
}

IntegrityTransactions::~IntegrityTransactions() {
    delete ui;
}

void IntegrityTransactions::requestTransactions() {
    allTrans.clear();
    nodeHeight = -1;

    ui->progressFrame->show();
    ui->transactionTable->hide();
    ui->transactionTable->clearAll(false);

    // !!! Note, order is important even it is async. We want node status be processed first..
    wallet->requestNodeStatus(); // Need to know th height.
    wallet->requestTransactions("integrity", true);
    updateData();
}

void IntegrityTransactions::updateData() {
    ui->transactionTable->clearAll(false);

    QDateTime current = QDateTime::currentDateTime();
    int txCnt = 0;

    for ( int idx = allTrans.size()-1; idx>=0 && txCnt<5000; idx--, txCnt++) {
        const wallet::WalletTransaction &trans = allTrans[idx];

        if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::CANCELLED)
            continue;
        // Don't want see see duplicates
        if ( trans.address == "Integrity fee" && (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::RECEIVE) )
            continue;

        // if the node is online and in sync, display the number of confirmations instead of time
        // trans.confirmationTime format: 2020-10-13 04:36:54
        // Expected: Jan 2, 2020 / 2:07am
        QString txTimeStr = trans.confirmationTime;
        if (txTimeStr.isEmpty() || txTimeStr == "None")
            txTimeStr = trans.creationTime;

        QDateTime txTime = QDateTime::fromString(txTimeStr, "HH:mm:ss dd-MM-yyyy");
        txTimeStr = txTime.toString("MMM d, yyyy / H:mmap");
        bool blocksPrinted = false;
        if (trans.confirmed && nodeHeight > 0 && trans.height > 0) {
            // confirmations are 1 more than the difference between the node and transaction heights
            int64_t confirmations = nodeHeight - trans.height + 1;
        }

        control::RichItem *itm = control::createMarkedItem(QString::number(idx), ui->transactionTable,
                                                           trans.canBeCancelled());

        { // First line
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
            // Adding Icon and a text
            itm->addWidget( control::createLabel(itm, false, false, "#" + QString::number(trans.txIdx + 1)) ).addFixedHSpacer(10);
            itm->addWidget( control::createLabel(itm, false, false, trans.address) );

            // Update with time or blocks
            itm->addHSpacer().addWidget(control::createLabel(itm, false, true, txTimeStr));

            itm->pop();
        } // First line

        itm->addWidget(control::createHorzLine(itm));

        control::RichButton *cancelBtn = nullptr;
        control::RichButton *repostBtn = nullptr;

        // Line with amount
        {
            QString amount;
            if (trans.address == "Integrity fee") {
                amount = "Paid fee " + util::nano2one(trans.fee);
            }
            else {
                amount = util::nano2one(trans.coinNano);
            }

            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
            itm->addWidget(control::createLabel(itm, false, false, amount + " MWC", control::FONT_LARGE));

            itm->addHSpacer();
            if (!blocksPrinted && nodeHeight > 0 && trans.height > 0) {
                itm->addWidget(control::createLabel(itm, false, true,
                                                    "Conf: " + QString::number(nodeHeight - trans.height + 1)));
            }
            itm->pop();
        }

        itm->apply();
        ui->transactionTable->addItem(itm);
    }
    ui->transactionTable->apply();
}

void IntegrityTransactions::on_backButton_clicked() {
    swapMarketplace->pageFee();
}

void IntegrityTransactions::onSgnTransactions( QString acc, QString height, QVector<QString> transactions)  {
    Q_UNUSED(height)

    if (acc != "integrity" )
        return;

    ui->progressFrame->hide();
    ui->transactionTable->show();

    allTrans.clear();

    for (QString & t : transactions ) {
        allTrans.push_back( wallet::WalletTransaction::fromJson(t) );
    }

    updateData();
}

void IntegrityTransactions::onSgnNodeStatus( bool online, QString errMsg, int _nodeHeight, int peerHeight, QString totalDifficulty, int connections ) {
    Q_UNUSED(errMsg);
    Q_UNUSED(peerHeight);
    Q_UNUSED(totalDifficulty);
    Q_UNUSED(connections);

    if (online)
        nodeHeight = _nodeHeight;
}

void IntegrityTransactions::onSgnNewNotificationMessage(int level, QString message) // level: bridge::MESSAGE_LEVEL values
{
    Q_UNUSED(level)
    if (message.contains("Changing transaction")) {
        requestTransactions();
    }
}


}

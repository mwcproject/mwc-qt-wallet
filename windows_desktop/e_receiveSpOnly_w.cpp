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

#include "e_receiveSpOnly_w.h"
#include "ui_e_receiveSpOnly.h"
#include <QFileInfo>
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/e_receive_b.h"
#include "../core/global.h"
#include "../dialogs_desktop/g_inputslatepackdlg.h"

namespace wnd {

ReceiveSpOnly::ReceiveSpOnly(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::ReceiveSpOnly)
{
    ui->setupUi(this);

    config  = new bridge::Config(this);
    wallet  = new bridge::Wallet(this);
    receive = new bridge::Receive(this);
    util    = new bridge::Util(this);

    QObject::connect( receive, &bridge::Receive::sgnTransactionActionIsFinished,
                      this, &ReceiveSpOnly::onSgnTransactionActionIsFinished, Qt::QueuedConnection);

    QObject::connect( wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
                      this, &ReceiveSpOnly::onSgnWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnFileProofAddress,
                      this, &ReceiveSpOnly::onSgnFileProofAddress, Qt::QueuedConnection);

    QObject::connect(wallet, &bridge::Wallet::sgnDecodeSlatepack,
                     this, &ReceiveSpOnly::onSgnDecodeSlatepack, Qt::QueuedConnection);

    ui->progress->initLoader(false);
    ui->slatepack_status->hide();
    ui->continueButton->setEnabled(false);

    updateAccountList();

    wallet->requestFileProofAddress();
}

ReceiveSpOnly::~ReceiveSpOnly() {
    delete ui;
}

void ReceiveSpOnly::onSgnTransactionActionIsFinished( bool success, QString message ) {
    util::TimeoutLockObject to( "ReceiveSpOnly" );

    ui->progress->hide();
    control::MessageBox::messageText(this, success ? "Success" : "Failure", message );
}

void ReceiveSpOnly::on_accountComboBox_activated(int index)
{
    Q_UNUSED(index);
    auto accountName = ui->accountComboBox->currentData(); // QVariant
    if (accountName.isValid())
        wallet->setReceiveAccount(accountName.toString());
}

void ReceiveSpOnly::updateAccountList() {
    // accountInfo - pairs of [name, longInfo]
    QVector<QString> accountInfo = wallet->getWalletBalance(true, false, true);
    QString selectedAccount = wallet->getReceiveAccount();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx=0;
    for (int i=1; i<accountInfo.size(); i+=2) {
        if ( accountInfo[i-1] == "integrity")
            continue;

        if (accountInfo[i-1] == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( accountInfo[i], QVariant(accountInfo[i-1]) );
        idx++;
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
}

void ReceiveSpOnly::onSgnWalletBalanceUpdated() {
    updateAccountList();
}

void ReceiveSpOnly::onSgnFileProofAddress(QString proofAddress) {
    ui->torAddress->setText(proofAddress);
}

void ReceiveSpOnly::onSgnDecodeSlatepack(QString tag, QString error, QString slatepack, QString slateJson, QString content, QString sender, QString recipient) {
    Q_UNUSED(recipient)

    if (tag != "ReceiveSpOnly")
        return;

    isSpValid = false;
    spInProgress = "";
    ui->slatepack_status->hide();
    if (!error.isEmpty()) {
        ui->slatepack_status->show();
        ui->slatepack_status->setText("<b>" + error + "</b>");
    }
    else {
        if ("SendInitial" != content) {
            ui->slatepack_status->show();
            ui->slatepack_status->setText( "<b>Wrong slatepack content. Expected SendInitial, but got " + content + "</b>" );
        }
        else {
            // Validating Json
            QVector<QString> slateParseRes = util->parseSlateContent(slateJson, int(util::FileTransactionType::RECEIVE), sender );
            Q_ASSERT(slateParseRes.size() == 1 || slateParseRes.size() >= 2);
            if (slateParseRes.size() == 1) {
                // parser reported error
                ui->slatepack_status->show();
                ui->slatepack_status->setText("<b>" + slateParseRes[0] + "</b>" );
            }
            else {
                QString spDesk;
                QString senderStr;
                if (sender == "None") {
                    spDesk = "non encrypted Slatepack";
                } else {
                    spDesk = "encrypted Slatepack";
                    senderStr = " from " + sender;
                }

                // mwc is on nano units
                QString mwcStr = util->nano2one(slateParseRes[1]);

                ui->slatepack_status->setText("You receive " + spDesk + " for " + mwcStr + " MWC" + senderStr);
                isSpValid = true;
                this->slatepack = slatepack;
                this->slateJson = slateJson;
                this->sender = sender;
                ui->slatepack_status->show();
            }
        }
    }

    QString textSp = ui->slatepackEdit->toPlainText().trimmed();

    if (slatepack != textSp) {
        initiateSlateVerification(textSp);
        isSpValid = false;
    }

    updateButtons();
}

void ReceiveSpOnly::on_slatepackEdit_textChanged() {
    isSpValid = false;
    updateButtons();

    if (spInProgress.isEmpty()) {
        QString sp = ui->slatepackEdit->toPlainText().trimmed();
        initiateSlateVerification(sp);
    }
}

// Accepting the slatepack and continue to process it
void ReceiveSpOnly::on_continueButton_clicked() {
    if (!isSpValid)
        return;

    ui->progress->show();
    receive->signSlatepackTransaction(slatepack, slateJson, sender );
}

void ReceiveSpOnly::updateButtons() {
    ui->continueButton->setEnabled(isSpValid);
}

void ReceiveSpOnly::initiateSlateVerification(const QString &slate2check) {
    if (slate2check.isEmpty()) {
        ui->slatepack_status->hide();
        return;
    }

    spInProgress = slate2check;
    wallet->decodeSlatepack(slate2check, "ReceiveSpOnly");
}

}


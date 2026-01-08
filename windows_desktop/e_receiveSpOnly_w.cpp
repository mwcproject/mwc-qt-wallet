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
#include <QJsonDocument>

#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/e_receive_b.h"
#include "../core/global.h"
#include "../dialogs_desktop/g_inputslatepackdlg.h"
#include "util/message_mapper.h"
#include "zz_utils.h"

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

    ui->progress->initLoader(false);
    ui->slatepack_status->hide();
    ui->continueButton->setEnabled(false);

    updateAccountsData(wallet, ui->accountComboBox, true, true);

    QString address = wallet->getTorSlatepackAddress();
    ui->torAddress->setText(address);
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
    auto accountPath = ui->accountComboBox->currentData(); // QVariant
    if (accountPath.isValid())
        wallet->setReceiveAccountById(accountComboData2AccountPath(accountPath.toString()).second);
}

void ReceiveSpOnly::onSgnWalletBalanceUpdated() {
    updateAccountsData(wallet, ui->accountComboBox, true, true);
}

void ReceiveSpOnly::on_slatepackEdit_textChanged() {
    isSpValid = false;
    updateButtons();

    QString sp = ui->slatepackEdit->toPlainText().trimmed();
    initiateSlateVerification(sp);
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

    QJsonObject decodeRes = wallet->decodeSlatepack(slate2check);
    wallet::DecodedSlatepack decodedSp = wallet::DecodedSlatepack::fromJson(decodeRes);

    isSpValid = false;
    ui->slatepack_status->hide();
    if (!decodedSp.error.isEmpty()) {
        ui->slatepack_status->show();
        ui->slatepack_status->setText("<b>" + decodedSp.error + "</b>");
    }
    else {
        if ("SendInitial" != decodedSp.content) {
            ui->slatepack_status->show();
            ui->slatepack_status->setText( "<b>Wrong slatepack content. Expected SendInitial, but got " + decodedSp.content + "</b>" );
        }
        else {
            // Validating Json

            QString slateAsStr = QJsonDocument(decodedSp.slate).toJson(QJsonDocument::Compact);

            QVector<QString> slateParseRes = util->parseSlateContent(slateAsStr, int(util::FileTransactionType::RECEIVE), decodedSp.sender );
            Q_ASSERT(slateParseRes.size() == 1 || slateParseRes.size() >= 2);
            if (slateParseRes.size() == 1) {
                // parser reported error
                ui->slatepack_status->show();
                ui->slatepack_status->setText("<b>" + slateParseRes[0] + "</b>" );
            }
            else {
                QString spDesk;
                QString senderStr;
                if (decodedSp.sender.isEmpty()) {
                    spDesk = "non encrypted Slatepack";
                } else {
                    spDesk = "encrypted Slatepack";
                    senderStr = " from " + decodedSp.sender;
                }

                // mwc is on nano units
                QString mwcStr = util->nano2one(slateParseRes[1]);

                ui->slatepack_status->setText("You receive " + spDesk + " for " + mwcStr + " MWC" + senderStr);
                isSpValid = true;
                this->slatepack = slate2check;
                this->slateJson = slateAsStr;
                this->sender = decodedSp.sender;
                ui->slatepack_status->show();
            }
        }
    }

    QString textSp = ui->slatepackEdit->toPlainText().trimmed();
    updateButtons();
}

}


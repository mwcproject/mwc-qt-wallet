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

#include "e_receive_w.h"
#include "ui_e_receive.h"
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

Receive::Receive(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::Receive)
{
    ui->setupUi(this);

    config  = new bridge::Config(this);
    wallet  = new bridge::Wallet(this);
    receive = new bridge::Receive(this);
    util    = new bridge::Util(this);

    QObject::connect( receive, &bridge::Receive::sgnTransactionActionIsFinished,
                      this, &Receive::onSgnTransactionActionIsFinished, Qt::QueuedConnection);

    QObject::connect( wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
                      this, &Receive::onSgnWalletBalanceUpdated, Qt::QueuedConnection);

    QObject::connect( wallet, &bridge::Wallet::sgnMwcAddressWithIndex,
                      this, &Receive::onSgnMwcAddressWithIndex, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnTorAddress,
                      this, &Receive::onSgnTorAddress, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnFileProofAddress,
                      this, &Receive::onSgnFileProofAddress, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnUpdateListenerStatus,
                      this, &Receive::onSgnUpdateListenerStatus, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    updateAccountList();

    if (config->isColdWallet()) {
        ui->frameQs->hide();
        ui->mwcmqAddress->hide();
    }

    if ( config->hasTls())
        ui->httpLabel->setText("Https");

    wallet->requestFileProofAddress();

    updateStatus();
}

Receive::~Receive() {
    delete ui;
}

void Receive::updateStatus() {
    // Recieve is a simple small page, we can update all without problems
    QString address = wallet->getMqsAddress();

    mwcAddress = "mwcmqs://" + address;
    ui->mwcmqAddress->setText( mwcAddress );

    bool mqsOnline = wallet->getMqsListenerStatus();
    ui->mwcmqStatusImg->setPixmap( QPixmap(mqsOnline ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->mwcmqStatusLabel->setText( mqsOnline ? "Online" : "Offline" );

    bool torOnline = wallet->getTorListenerStatus();
    ui->torStatusImg->setPixmap( QPixmap(torOnline ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->torStatusLabel->setText( torOnline ? "Online" : "Offline" );

    bool httpOnline = wallet->getHttpListeningStatus()=="true";
    ui->httpStatusImg->setPixmap( QPixmap(httpOnline ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->httpStatusLabel->setText( httpOnline ? "Online" : "Offline" );
}


void Receive::on_recieveFileButton_clicked()
{
    util::TimeoutLockObject to( "Receive" );

    QString fileName = util->getOpenFileName( "Open initial transaction file", "fileGen", "MWC transaction (*.tx *.input);;All files (*.*)");
    if (fileName.isEmpty())
        return;

    ui->progress->show();

    receive->signTransaction(fileName);
    // Expected respond from state with result
}


void Receive::on_recieveSlatepackButton_clicked()
{
    util::TimeoutLockObject to( "Receive" );

    dlg::InputSlatepackDlg inputSlateDlg("SendInitial", "Initial Send Slate",
                           util::FileTransactionType::RECEIVE,  this);

    if (inputSlateDlg.exec()  == QDialog::Accepted ) {
        ui->progress->show();

        receive->signSlatepackTransaction(inputSlateDlg.getSlatepack(), inputSlateDlg.getSlateJson(), inputSlateDlg.getSenderAddress() );
    }
}


void Receive::onSgnTransactionActionIsFinished( bool success, QString message ) {
    util::TimeoutLockObject to( "Receive" );

    ui->progress->hide();
    control::MessageBox::messageText(this, success ? "Success" : "Failure", message );
}

void Receive::on_accountComboBox_activated(int index)
{
    Q_UNUSED(index);
    auto accountName = ui->accountComboBox->currentData(); // QVariant
    if (accountName.isValid())
        wallet->setReceiveAccount(accountName.toString());
}

void Receive::updateAccountList() {
    // accountInfo - pairs of [name, longInfo]
    QVector<QString> accountInfo = wallet->getWalletBalance(true, false, true);
    QString selectedAccount = wallet->getReceiveAccount();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx=0;
    for (int i=1; i<accountInfo.size(); i+=2) {
        if (accountInfo[i-1] == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( accountInfo[i], QVariant(accountInfo[i-1]) );
        idx++;
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
}

void Receive::onSgnWalletBalanceUpdated() {
    updateAccountList();
}

void Receive::onSgnMwcAddressWithIndex(QString mwcAddress, int idx) {
    Q_UNUSED(idx);
    Q_UNUSED(mwcAddress);
    updateStatus();
}
void Receive::onSgnTorAddress(QString tor) {
    Q_UNUSED(tor);
    updateStatus();
}

void Receive::onSgnFileProofAddress(QString proofAddress) {
    ui->fileAddress->setText(proofAddress);
}

// keybaseOnline is absolete
void Receive::onSgnUpdateListenerStatus(bool mqsOnline, bool keybaseOnline, bool torOnline) {
    Q_UNUSED(mqsOnline)
    Q_UNUSED(keybaseOnline)
    Q_UNUSED(torOnline)
    updateStatus();
}


}


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

#include "e_recieve_w.h"
#include "ui_e_recieve.h"
#include <QFileInfo>
#include "../state/e_Recieve.h"
#include <QFileDialog>
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"

namespace wnd {

Recieve::Recieve(QWidget *parent, state::Recieve * _state, bool mwcMqStatus, bool keybaseStatus,
                 QString mwcMqAddress) :
        core::NavWnd(parent, _state->getContext() ),
        ui(new Ui::Recieve),
        state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    updateAccountList();

    updateMwcMqState(mwcMqStatus);
    updateKeybaseState(keybaseStatus);
    updateMwcMqAddress(mwcMqAddress);
}

Recieve::~Recieve() {
    state->deletedWnd(this);
    delete ui;
}

void Recieve::updateMwcMqAddress(QString address) {
    mwcAddress = "mwcmq://" + address;
    ui->mwcmqAddress->setText( mwcAddress );
}

void Recieve::updateMwcMqState(bool online) {
    ui->mwcmqStatusImg->setPixmap( QPixmap(online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->mwcmqStatusLabel->setText( online ? "Online" : "Offline" );
}

void Recieve::updateKeybaseState(bool online) {
    ui->keybaseStatusImg->setPixmap( QPixmap(online ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->keybaseStatusLabel->setText( online ? "Online" : "Offline" );
}

void Recieve::on_pushButton_clicked() {
    state::TimeoutLockObject to( state );

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open initial transaction file"),
                                                    state->getFileGenerationPath(),
                                                    tr("MWC transaction (*.tx)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath( flInfo.path() );

    ui->progress->show();

    state->signTransaction(fileName);
    // Expected respond from state with result
}

void Recieve::onTransactionActionIsFinished( bool success, QString message ) {
    state::TimeoutLockObject to( state );

    ui->progress->hide();
    control::MessageBox::message(this, success ? "Success" : "Failure", message );
}

void Recieve::on_accountComboBox_activated(int index)
{
    if (index>=0 && index < accountInfo.size() )
        state->setReceiveAccount( accountInfo[index].accountName );
}

void Recieve::updateAccountList() {
    accountInfo = state->getWalletBalance();
    QString selectedAccount = state->getReceiveAccount();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx=0;
    for (auto & info : accountInfo) {
        if (info.accountName == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( info.getLongAccountName(), QVariant(idx++) );
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
}

}


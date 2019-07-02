#include "e_recieve_w.h"
#include "ui_e_recieve.h"
#include <QFileInfo>
#include "../state/e_Recieve.h"
#include <QFileDialog>
#include "../control/messagebox.h"

namespace wnd {

Recieve::Recieve(QWidget *parent, state::Recieve * _state, bool mwcMqStatus, bool keybaseStatus,
                 QString mwcMqAddress) :
        core::NavWnd(parent, _state->getStateMachine() ),
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
    state->deletedWnd();
    delete ui;
}

void Recieve::updateMwcMqAddress(QString address) {
    ui->mwcmqAddress->setText( "mwcmq://" + address );
}

void Recieve::updateMwcMqState(bool online) {
    ui->mwcmqStatusImg->setPixmap( QPixmap(online ? ":/img/StatusOk.png" : ":/img/StatusEmpty.png") );
    ui->mwcmqStatusLabel->setText( online ? "Online" : "Offline" );
}

void Recieve::updateKeybaseState(bool online) {
    ui->keybaseStatusImg->setPixmap( QPixmap(online ? ":/img/StatusOk.png" : ":/img/StatusEmpty.png") );
    ui->keybaseStatusLabel->setText( online ? "Online" : "Offline" );
}

void Recieve::on_pushButton_clicked() {
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

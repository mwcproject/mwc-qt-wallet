#include "e_recieve_w.h"
#include "ui_e_recieve.h"
#include <QFileInfo>
#include "../state/e_Recieve.h"
#include <QFileDialog>
#include "../control/messagebox.h"

namespace wnd {

Recieve::Recieve(QWidget *parent, state::Recieve * _state, bool mwcMqStatus, bool keybaseStatus,
                 QString mwcMqAddress) :
        QWidget(parent),
        ui(new Ui::Recieve),
        state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

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
    ui->keybaseStatusImg->setPixmap( QPixmap( online ? ":/img/StatusOk.png" : ":/img/StatusEmpty.png" ) );
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


}

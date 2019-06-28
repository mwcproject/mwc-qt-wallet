#include "e_listening_w.h"
#include "ui_e_listening.h"
#include "../state/e_listening.h"
#include <QInputDialog>
#include "../control/messagebox.h"
#include <QDebug>

namespace wnd {

Listening::Listening(QWidget *parent, state::Listening * _state, bool mwcMqStatus, bool keybaseStatus,
                     QString mwcMqAddress, int mwcMqAddrIdx) :
    QWidget(parent),
    ui(new Ui::Listening),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Listening");

    updateMwcMqState(mwcMqStatus);
    updateKeybaseState(keybaseStatus);
    updateMwcMqAddress(mwcMqAddress, mwcMqAddrIdx);
}

Listening::~Listening()
{
    state->wndIsGone();
    delete ui;
}

void Listening::showMessage(QString title, QString message) {
    control::MessageBox::message(this, title, message);
}


void Listening::updateMwcMqAddress(QString address, int addrIdx) {

    ui->mwcMqAddress->setText( address );
    ui->mwcMqAddressIndexLabel->setText( addrIdx>=0 ? ("Address Index: " + QString::number(addrIdx)) : "" );
}

void Listening::updateMwcMqState(bool online) {
    ui->mwcMqStatusImg->setPixmap( QPixmap(online ? ":/img/StatusOk.png" : ":/img/StatusEmpty.png") );
    ui->mwcMqStatusImg->setToolTip(online ? "Listener connected to mwcmq" : "Listener diconnected from mwcmq");
    ui->mwcMqStatusTxt->setText( online ? "Online" : "Offline" );
    ui->mwcMqTriggerButton->setText( online ? "Stop" : "Start" );
    ui->mwcMqTriggerButton->setToolTip(online ? "Stop the MWC MQ Listener" : "Start the MWC MQ Listener");
    ui->mwcMqNextAddress->setEnabled(!online);
    ui->mwcMqToIndex->setEnabled(!online);
}

void Listening::updateKeybaseState(bool online) {

    ui->keybaseStatusImg->setPixmap( QPixmap( online ? ":/img/StatusOk.png" : ":/img/StatusEmpty.png" ) );
    ui->keybaseStatusImg->setToolTip(online ? "Listener connected to keybase" : "Listener diconnected from keybase");
    ui->keybaseStatusTxt->setText( online ? "Online" : "Offline" );
    ui->keybaseTriggerButton->setToolTip(online ? "Stop the Keybase Listener" : "Start the Keybase Listener");
    ui->keybaseTriggerButton->setText( online ? "Stop" : "Start" );
}

void Listening::on_mwcMqTriggerButton_clicked()
{
    state->triggerMwcState();
}

void Listening::on_mwcMqNextAddress_clicked()
{
    state->requestNextMwcMqAddress();

}

void Listening::on_mwcMqToIndex_clicked()
{
    bool ok = false;
    QString index = QInputDialog::getText(this, tr("Select MWX box address by index"),
                                            tr("Please specify index of mwc mq address"), QLineEdit::Normal,
                                            "", &ok);
    if (!ok || index.isEmpty())
        return;

    ok = false;
    int idx = index.toInt(&ok);

    if (!ok || idx<0 || idx>65000) {
        control::MessageBox::message(this, "Wrong value", "Please input integer on the range from 0 to 65000");
        return;
    }

    state->requestNextMwcMqAddressForIndex(idx);
}

void Listening::on_keybaseTriggerButton_clicked()
{
    state->triggerKeybaseState();
}


}




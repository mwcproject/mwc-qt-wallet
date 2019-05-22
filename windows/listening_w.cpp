#include "listening_w.h"
#include "ui_listening.h"
#include "../state/listening.h"
#include <QInputDialog>
#include <QMessageBox>

namespace wnd {

Listening::Listening(QWidget *parent, state::Listening * _state) :
    QWidget(parent),
    ui(new Ui::Listening),
    state(_state)
{
    ui->setupUi(this);

    ui->httpGroupBox->hide();

    state->setWindowTitle("Listening");

    updateBoxState();
    updateKeystoneState();
    updateHttpForeignApiState();


}

Listening::~Listening()
{
    delete ui;
}

void Listening::updateBoxState() {
    QPair<bool, QString> status = state->getBoxListeningStatus();
    if (status.first) {
        ui->mwcBoxStatus->setText( "Online" +  (status.second.length()==0 ? "" : (", "+ status.second)));
        ui->mwcBoxConnectBtn->setText( "Stop" );

        QPair<QString,int> boxAddress = state->getBoxAddress();

        ui->mwcBoxAddress->setText( boxAddress.first );
        ui->mwcBoxAddressIdx->setText( "Address (" + QString::number(boxAddress.second) + "):" );
    }
    else {
        ui->mwcBoxConnectBtn->setText( "Start" );
        ui->mwcBoxStatus->setText( "Offline" );
        ui->mwcBoxAddress->setText("N/A");
        ui->mwcBoxAddressIdx->setText("Address:");
    }
}

void Listening::updateKeystoneState() {
    QPair<bool, QString> status = state->getKeystoneListeningStatus();

    if (status.first) {
        ui->keystoneStatus->setText( "Online" +  (status.second.length()==0 ? "" : (", "+status.second)));
        ui->keystoneConnectBtn->setText( "Stop" );
    }
    else {
        ui->keystoneStatus->setText("Offline");
        ui->keystoneConnectBtn->setText("Start");
    }
}

void Listening::updateHttpForeignApiState() {
    // Not implemented
}


void Listening::on_nextAddressButton_clicked()
{
    state->nextBoxAddress();
    updateBoxState();
}

void Listening::on_mwcAddressFromIndexButton_clicked()
{
    bool ok = false;
    QString index = QInputDialog::getText(this, tr("Select MWX box address by index"),
                                            tr("Please specify index of select that MWC box address"), QLineEdit::Normal,
                                            "", &ok);
    if (!ok || index.isEmpty())
        return;

    ok = false;
    int idx = index.toInt(&ok);

    if (!ok || idx<0 || idx>65000) {
        QMessageBox::critical(this, "Wrong value", "Please input integer on the range from 0 to 65000");
        return;
    }

    state->changeBoxAddress(idx);
    updateBoxState();
}

void Listening::on_mwcBoxConnectBtn_clicked()
{
    QPair<bool, QString> status = state->getBoxListeningStatus();
    if (status.first) {
        // stop listening
        QPair<bool, QString> res = state->stopBoxListeningStatus();
        if (!res.first)
            QMessageBox::critical(this,"Listening error", "Unable to process stop listening request. Error: " + res.second);
    }
    else {
        // start listening
        QPair<bool, QString> res = state->startBoxListeningStatus();
        if (!res.first)
            QMessageBox::critical(this,"Listening error", "Unable to process satrt listening request. Error: " + res.second);
    }

    updateBoxState();
}

void Listening::on_keystoneConnectBtn_clicked()
{
    QPair<bool, QString> status = state->getKeystoneListeningStatus();
    if (status.first) {
        // stop listening
        QPair<bool, QString> res = state->stopKeystoneListeningStatus();
        if (!res.first)
            QMessageBox::critical(this,"Listening error", "Unable to process stop listening request. Error: " + res.second);
    }
    else {
        // start listening
        QPair<bool, QString> res = state->startKeystoneListeningStatus();
        if (!res.first)
            QMessageBox::critical(this,"Listening error", "Unable to process satrt listening request. Error: " + res.second);
    }

    updateKeystoneState();
}

void Listening::on_foreignConnectBtn_clicked()
{
    // not implemented
}

}


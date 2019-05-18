#include "windows/airdropforbtc_w.h"
#include "ui_airdropforbtc.h"
#include "../state/airdrop.h"
#include <QMessageBox>

namespace wnd {

AirdropForBTC::AirdropForBTC(QWidget *parent, state::Airdrop * _state) :
    QWidget(parent),
    ui(new Ui::AirdropForBTC),
    state(_state)
{
    ui->setupUi(this);

    state::AirdropChallendge currentClaim = state->getLastAirdropChallendge();

    ui->btcEdit->setText(currentClaim.btcAddress);
    ui->challengeEdit->setText(currentClaim.challendge);
}

AirdropForBTC::~AirdropForBTC()
{
    delete ui;
}

void AirdropForBTC::on_claimButton_clicked()
{
    QString btcAddr = ui->btcEdit->text();
    QString challenge = ui->challengeEdit->text();
    QString signature = ui->signatureEdit->text();

    if ( signature.length() == 0 ) {
        QMessageBox::critical(this, "MWC claim", "Please input the signature for the challenge");
        return;
    }

    QPair<bool, QString> res = state->requestMWC( btcAddr, challenge, signature );
    if (!res.first) {
        QMessageBox::critical(this, "MWC claim", "Your MWC claim request failed. Error: " + res.second);
        return;
    }
    else {
        if (res.second.length()>0)
            QMessageBox::information(this, "MWC claim", "Your MWC claim request was processed successfully. " + res.second);
        else
            QMessageBox::information(this, "MWC claim", "Your MWC claim request was processed successfully.");

        state->backToMainAirDropPage();
    }
}

void AirdropForBTC::on_backButton_clicked()
{
    state->backToMainAirDropPage();
}

}

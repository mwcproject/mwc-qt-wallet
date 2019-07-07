#include "windows/airdropforbtc_w.h"
#include "ui_airdropforbtc.h"
#include "../state/airdrop.h"
#include "../control/messagebox.h"

namespace wnd {

AirdropForBTC::AirdropForBTC(QWidget *parent, state::Airdrop * _state) :
    QWidget(parent),
    ui(new Ui::AirdropForBTC),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Proof your BTC ownership");

    state::AirdropChallendge currentClaim = state->getLastAirdropChallendge();

    ui->btcEdit->setText(currentClaim.btcAddress);
    ui->challengeEdit->setText(currentClaim.challendge);

    ui->signatureEdit->setFocus();
}

AirdropForBTC::~AirdropForBTC()
{
    delete ui;
}

void AirdropForBTC::on_claimButton_clicked()
{
    QString btcAddr = ui->btcEdit->text().trimmed();
    QString challenge = ui->challengeEdit->text().trimmed();
    QString signature = ui->signatureEdit->text().trimmed();

    if ( signature.length() == 0 ) {
        control::MessageBox::message(this, "MWC claim", "Please input the signature for the challenge");
        return;
    }

    QPair<bool, QString> res = state->requestMWC( btcAddr, challenge, signature );
    if (!res.first) {
        control::MessageBox::message(this, "Claim failure", "Your MWC claim request failed.\nError: " + res.second);
        return;
    }
    else {
        if (res.second.length()>0)
            control::MessageBox::message(this, "Success", "Your MWC claim request was processed successfully. " + res.second);
        else
            control::MessageBox::message(this, "Success", "Your MWC claim request was processed successfully.");

        state->backToMainAirDropPage();
    }
}

void AirdropForBTC::on_backButton_clicked()
{
    state->backToMainAirDropPage();
}

}

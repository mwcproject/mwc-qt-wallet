#include "windows/m_airdropforbtc_w.h"
#include "ui_m_airdropforbtc.h"
#include "state/m_airdrop.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"

namespace wnd {

AirdropForBTC::AirdropForBTC(QWidget *parent, state::Airdrop * _state, QString _btcAddress, QString _challenge, QString _identifier) :
    QWidget(parent),
    ui(new Ui::AirdropForBTC),
    state(_state),
    btcAddress(_btcAddress.trimmed()),
    challenge(_challenge.trimmed()),
    identifier(_identifier)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    ui->btcEdit->setText(btcAddress);
    ui->challengeEdit->setText(challenge);

    ui->signatureEdit->setFocus();
}

AirdropForBTC::~AirdropForBTC()
{
    state->deleteAirdropForBtcWnd(this);
    delete ui;
}

void AirdropForBTC::on_claimButton_clicked()
{
    state::TimeoutLockObject to( state );

    QString signature = ui->signatureEdit->toPlainText().trimmed();

    if ( signature.length() == 0 ) {
        control::MessageBox::message(this, "MWC claim", "Please input the signature for the challenge");
        return;
    }

    state->requestClaimMWC( btcAddress, challenge, signature, identifier );

    ui->progress->show();
}

void AirdropForBTC::on_backButton_clicked()
{
    state->backToMainAirDropPage();
}

void AirdropForBTC::reportMessage( QString title, QString message ) {
    state::TimeoutLockObject to( state );

    ui->progress->hide();
    control::MessageBox::message(this, title, message);
}


}

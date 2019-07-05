#include "send3_offlinesettings_w.h"
#include "ui_send3_offlinesettings.h"
#include "sendcoinsparamsdialog.h"
#include "state/send3_Offline.h"

namespace wnd {

SendOfflineSettings::SendOfflineSettings(QWidget *parent, state::SendOffline * _state) :
    core::NavWnd(parent, _state->getStateMachine(), _state->getAppContext() ),
    ui(new Ui::SendOfflineSettings),
    state(_state)
{
    ui->setupUi(this);

    accountInfo = state->getWalletBalance();

    int idx=0;
    for (auto & info : accountInfo) {
        ui->accountComboBox->addItem( util::expandStrR(info.accountName, 25) + " Available: " + util::nano2one(info.currentlySpendable) + " mwc", QVariant(idx++) );
    }
}

SendOfflineSettings::~SendOfflineSettings()
{
    state->deletedSendOfflineSettings();
    delete ui;
}

void SendOfflineSettings::on_settingsButton_clicked()
{
    core::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }

}

void SendOfflineSettings::on_sendButton_clicked()
{
    int accountIdx = ui->accountComboBox->currentData().toInt();
    wallet::AccountInfo acc = accountInfo[accountIdx];

    state->prepareSendMwcOffline( acc, ui->descriptionEdit->toPlainText() );

}

}

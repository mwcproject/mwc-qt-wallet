#include "send3_offlinesettings_w.h"
#include "ui_send3_offlinesettings.h"
#include "../dialogs/sendcoinsparamsdialog.h"
#include "state/send3_Offline.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"

namespace wnd {

SendOfflineSettings::SendOfflineSettings(QWidget *parent, state::SendOffline * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::SendOfflineSettings),
    state(_state)
{
    ui->setupUi(this);

    accountInfo = state->getWalletBalance();
    QString selectedAccount = state->getCurrentAccountName();

    int selectedAccIdx = 0;
    int idx=0;
    for (auto & info : accountInfo) {
        if (info.accountName == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( util::expandStrR(info.accountName, 25) + " Available: " + util::nano2one(info.currentlySpendable) + " mwc", QVariant(idx++) );
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
}

SendOfflineSettings::~SendOfflineSettings()
{
    state->deletedSendOfflineSettings(this);
    delete ui;
}

void SendOfflineSettings::on_settingsButton_clicked()
{
    state::TimeoutLockObject to( state );

    core::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }

}

void SendOfflineSettings::on_sendButton_clicked()
{
    state::TimeoutLockObject to( state );

    int accountIdx = ui->accountComboBox->currentData().toInt();
    wallet::AccountInfo acc = accountInfo[accountIdx];

    QString description = ui->descriptionEdit->toPlainText().trimmed();

    {
        QPair<bool, QString> valRes = util::validateMwc713Str(description);
        if (!valRes.first) {
            control::MessageBox::message(this, "Incorrect Input", valRes.second);
            ui->descriptionEdit->setFocus();
            return;
        }
    }

    state->prepareSendMwcOffline( acc, description );
}


}

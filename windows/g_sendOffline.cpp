#include "g_sendOffline.h"
#include "ui_g_sendOffline.h"
#include "../dialogs/sendcoinsparamsdialog.h"
#include "state/g_Send.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include <QFileDialog>

namespace wnd {

SendOffline::SendOffline(QWidget *parent, const wallet::AccountInfo & _selectedAccount, int64_t _amount, state::Send * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::SendOffline),
    state(_state),
    selectedAccount(_selectedAccount),
    amount(_amount)
{
    ui->setupUi(this);
    ui->progress->initLoader(false);

    ui->fromAccount->setText("From account: " + selectedAccount.accountName );
    ui->amount2send->setText( "Amount to send: " + util::nano2one(amount) + " mwc" );

}

SendOffline::~SendOffline()
{
    state->destroyOfflineWnd(this);
    delete ui;
}

void SendOffline::on_settingsButton_clicked()
{
    state::TimeoutLockObject to( state );

    core::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }

}

void SendOffline::on_sendButton_clicked()
{
    state::TimeoutLockObject to( state );

    QString description = ui->descriptionEdit->toPlainText().trimmed();

    {
        QPair<bool, QString> valRes = util::validateMwc713Str(description);
        if (!valRes.first) {
            control::MessageBox::message(this, "Incorrect Input", valRes.second);
            ui->descriptionEdit->setFocus();
            return;
        }
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Create initial transaction file"),
                                                    state->getFileGenerationPath(),
                                                    tr("MWC init transaction (*.tx)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath( flInfo.path() );

    ui->progress->show();

    state->sendMwcOffline(  selectedAccount, amount, description, fileName );
}

void SendOffline::showSendMwcOfflineResult( bool success, QString message ) {
    state::TimeoutLockObject to( state );

    ui->progress->hide();
    control::MessageBox::message(this, success ? "Success" : "Failure", message );
}



}

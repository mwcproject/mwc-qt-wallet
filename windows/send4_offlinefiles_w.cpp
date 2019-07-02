#include "send4_offlinefiles_w.h"
#include "send2_online_w.h"
#include "ui_send4_offlinefiles.h"
#include "../util/stringutils.h"
#include "../control/messagebox.h"
#include "../state/send3_Offline.h"
#include <QFileDialog>

namespace wnd {

SendOfflineFiles::SendOfflineFiles(QWidget *parent, const wallet::AccountInfo & _account, state::SendOffline * _state) :
    core::NavWnd(parent, _state->getStateMachine() ),
    ui(new Ui::SendOfflineFiles),
    state(_state),
    account(_account)
{
    ui->setupUi(this);
    ui->progress->initLoader(false);
}

SendOfflineFiles::~SendOfflineFiles()
{
    state->deletedSendOfflineFiles();
    delete ui;
}

void SendOfflineFiles::on_generateFileButton_clicked()
{
    QString mwc2sendStr = ui->amountEdit->text();
    auto mwcAmount = util::one2nano( mwc2sendStr );
    if (!mwcAmount.first) {
        control::MessageBox::message(this, "Verification Error", "Please input number of MWC coins that you want to send");
        return;
    }


    if ( mwcAmount.second > account.currentlySpendable ) {

        QString msg2print = generateAmountErrorMsg( mwcAmount.second, account, state->getSendCoinsParams() );

        control::MessageBox::message(this, "Incorrect Input",
                                     msg2print );
        ui->amountEdit->setFocus();
        return;
    }

    long nanoCoins = mwcAmount.second;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Create initial transaction file"),
                                                          state->getFileGenerationPath(),
                                                          tr("MWC init transaction (*.tx)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath( flInfo.path() );

    ui->progress->show();

    state->sendToFile(nanoCoins, fileName);
    // Wallet should answer with a callback.
}

/*void SendOfflineFiles::on_signTransactionButton_clicked()
{
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
}*/

void SendOfflineFiles::on_publishTransactionButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open signed transaction file"),
                                                          state->getFileGenerationPath(),
                                                            tr("MWC transaction (*.tx.response)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath( flInfo.path() );

    ui->progress->show();

    state->publishTransaction(fileName);
    // Expected respond from state with result
}

void SendOfflineFiles::onTransactionActionIsFinished( bool success, QString message ) {
    ui->progress->hide();
    control::MessageBox::message(this, success ? "Success" : "Failure", message );
}


}



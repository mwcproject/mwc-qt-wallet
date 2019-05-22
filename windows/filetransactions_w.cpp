#include "filetransactions_w.h"
#include "ui_filetransactions.h"
#include "../state/filetransactions.h"
#include <QFileDialog>
#include "../util/stringutils.h"
#include <QMessageBox>

namespace wnd {

FileTransactions::FileTransactions(QWidget *parent, state::FileTransactions * _state) :
    QWidget(parent),
    ui(new Ui::FileTransactions),
    state(_state)
{
    ui->setupUi(this);
    state->setWindowTitle("Send/Recieve MWC offline");
}

FileTransactions::~FileTransactions()
{
    delete ui;
}

void FileTransactions::on_generateSendCoinsFileButton_clicked()
{
    QString mwc2sendStr = ui->sendCoinsEdit->text();
    auto res = util::one2nano( mwc2sendStr );
    if (!res.first) {
        QMessageBox::critical(this, "MWC coints input", "Please input number of MWC coins that you want to send");
        return;
    }
    long nanoCoins = res.second;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Create initial transaction file"),
                                                          state->getFileGenerationPath(),
                                                            tr("MWC init transaction (*.tx)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath( flInfo.path() );


    QPair<bool, QString> sendRes = state->generateSendCoinsFile(nanoCoins, fileName);
    if (sendRes.first) {
        QMessageBox::information(this, "Send MWC coins",
                 "You sucessfully created a initial transaction file that send " +
                 mwc2sendStr + " MWC. The resulting file is located at " + flInfo.absoluteFilePath() );

        ui->sendCoinsEdit->clear();
        return;
    }
    else {
        QMessageBox::critical(this, "Send MWC coins",
                 "Unable to create initial transaction file. Error: " + sendRes.second );
        return;
    }

}

void FileTransactions::on_signTransactionButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open initial transaction file"),
                                                          state->getFileGenerationPath(),
                                                            tr("MWC transaction (*.tx)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath( flInfo.path() );

    QString resultFileName = fileName + ".response";

    QPair<bool, QString> sendRes = state->signTransaction(fileName, resultFileName);
    if (sendRes.first) {
        QMessageBox::information(this, "Sign transaction",
                 "You sucessfully signed transaction from sender. "
                 "The response file is located at " + resultFileName );
        return;
    }
    else {
        QMessageBox::critical(this, "Sign transaction",
                 "Sign transaction was failed. Error: " + sendRes.second );
        return;
    }
}

void FileTransactions::on_processResponceFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open your response file"),
                                                          state->getFileGenerationPath(),
                                                            tr("MWC response (*.response)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath( flInfo.path() );

    QPair<bool, QString> sendRes = state->processResponseFile(fileName);
    if (sendRes.first) {
        QMessageBox::information(this, "Process transaction",
                 "You sucessfully process and publish transaction from " + fileName );
        return;
    }
    else {
        QMessageBox::critical(this, "Process transaction",
                 "Transaction processing was failed. Error: " + sendRes.second );
        return;
    }
}

}



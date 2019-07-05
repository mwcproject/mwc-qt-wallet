#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDebug>
#include "versiondialog.h"
#include "../state/statemachine.h"
#include "util/widgetutils.h"
#include "../control/messagebox.h"

namespace core {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->statusBar->addPermanentWidget(ui->helpButton);
    ui->statusBar->addPermanentWidget(ui->btnSpacerLabel1);
    ui->statusBar->addPermanentWidget(ui->connectionStatusButton);
    ui->statusBar->addPermanentWidget(ui->rightestSpacerLabel);

    //ui->statusBar->showMessage("Can show any message here", 2000);

    // Want to delete children when they close
    setAttribute( Qt::WA_DeleteOnClose, true );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message) {

    using namespace wallet;

    QString prefix;
    int timeout = 3000;
    switch(level) {
    case WalletNotificationMessages::ERROR:
        prefix = "Error: ";
        timeout = 7000;
        break;
    case WalletNotificationMessages::WARNING:
        prefix = "Warning: ";
        timeout = 7000;
        break;
    case WalletNotificationMessages::INFO:
        prefix = "Info: ";
        timeout = 4000;
        break;
    case WalletNotificationMessages::DEBUG:
        prefix = "Debug: ";
        timeout = 2000;
        break;
    }

    ui->statusBar->showMessage( prefix + message, timeout);
}


void MainWindow::updateLeftBar(bool show) {
    if (leftBarShown == show)
        return;

    if (show) {
        ui->leftTb->show();
        ui->statusBar->show();
    }
    else {
        ui->leftTb->hide();
        ui->statusBar->hide();
    }
    leftBarShown = show;
}

void MainWindow::setAppEnvironment(state::StateMachine * _stateMachine, wallet::Wallet * _wallet) {
    stateMachine = _stateMachine;
    Q_ASSERT(stateMachine);
    wallet = _wallet;
    Q_ASSERT(wallet);

    ui->leftTb->setAppEnvironment(stateMachine, wallet);

    QObject::connect(wallet, &wallet::Wallet::onNewNotificationMessage,
                     this, &MainWindow::onNewNotificationMessage, Qt::QueuedConnection);


    QObject::connect(wallet, &wallet::Wallet::onMwcMqListenerStatus,
                     this, &MainWindow::updateListenerStatus, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onMwcMqListenerStatus,
                     this, &MainWindow::updateListenerStatus, Qt::QueuedConnection);

    updateListenerBtn();
}

QWidget * MainWindow::getMainWindow() {
    return ui->mainWindowFrame;
}

void MainWindow::updateActionStates(state::STATE actionState) {

    bool isLeftBarVisible = (actionState >= state::STATE::ACCOUNTS);

    updateLeftBar( isLeftBarVisible );

    if (isLeftBarVisible) {
        ui->leftTb->updateButtonsState(actionState);
    }

//    bool enabled = stateMachine->isActionWindowMode();

}

void MainWindow::on_connectionStatusButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::LISTENING );
}

void MainWindow::on_helpButton_clicked()
{
    control::MessageBox::message(this, "Help", "Here is we suppose provide some help." );
}

void MainWindow::updateListenerStatus(bool online) {
    Q_UNUSED(online);

    updateListenerBtn();
}

void MainWindow::updateListenerBtn() {
    QPair<bool,bool> listStatus = wallet->getListeningStatus();
    qDebug() << "updateListenerBtn: " << listStatus;


    bool listening = listStatus.first | listStatus.second;

    QPixmap pixmap( listening ? ":/img/StatusOk.png" : ":/img/StatusFail.png" );

    QIcon ButtonIcon(pixmap);
    ui->connectionStatusButton->setIcon( ButtonIcon );
    ui->connectionStatusButton->setToolTip(listening ? "You are listening. Click here to view listener status"
                                          : "You are not listening. Click here to view listener status");
}


/*void MainWindow::on_actionVersion_triggered()
{
    VersionDialog verDlg(this);
    verDlg.exec();
}

void MainWindow::on_actionAccount_triggered()
{
    stateMachine->setActionWindow( state::STATE::ACCOUNTS );
}

void MainWindow::on_actionEvents_triggered()
{
    stateMachine->setActionWindow( state::STATE::EVENTS );
}

void MainWindow::on_actionHODL_triggered()
{
    stateMachine->setActionWindow( state::STATE::HODL );
}

void MainWindow::on_actionSend_coins_triggered()
{
    stateMachine->setActionWindow( state::STATE::SEND_COINS );
}

void MainWindow::on_actionRecieve_coins_triggered()
{
    stateMachine->setActionWindow( state::STATE::RECIEVE_COINS );
}

void MainWindow::on_actionNode_status_triggered()
{
    stateMachine->setActionWindow( state::STATE::NODE_STATUS );
}

void MainWindow::on_actionSend_recieve_offline_triggered()
{
    stateMachine->setActionWindow( state::STATE::FILE_TRANSACTIONS );
}

void MainWindow::on_actionListening_Status_triggered()
{
    stateMachine->setActionWindow( state::STATE::LISTENING );
}

void MainWindow::on_actionTransactions_triggered()
{
    stateMachine->setActionWindow( state::STATE::TRANSACTIONS );
}

void MainWindow::on_actionOutputs_triggered()
{
    stateMachine->setActionWindow( state::STATE::OUTPUTS );
}

void MainWindow::on_actionContacts_triggered()
{
    stateMachine->setActionWindow( state::STATE::CONTACTS );
}

void MainWindow::on_actionConfig_triggered()
{
    stateMachine->setActionWindow( state::STATE::WALLET_CONFIG );
}

void core::MainWindow::on_actionAirdrop_triggered()
{
    stateMachine->setActionWindow( state::STATE::AIRDRDOP_MAIN );
}*/


}


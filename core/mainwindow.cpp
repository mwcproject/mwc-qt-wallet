#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include "versiondialog.h"
#include "../state/statemachine.h"

namespace core {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->statusBar->addPermanentWidget(ui->currentTime);
    ui->statusBar->showMessage("Can show any message here");

    clockTimer = new QTimer(this);
    connect(clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()) );
    clockTimer->start(1000);

    // Want to delete children when they close
    setAttribute( Qt::WA_DeleteOnClose, true );

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setStateMachine(state::StateMachine * _stateMachine) {
    stateMachine = _stateMachine;
    Q_ASSERT(stateMachine);
}


QWidget * MainWindow::getMainWindow() {
    return ui->mainWindowFrame;
}

void MainWindow::updateActionStates() {

    state::STATE state = stateMachine->getActionWindow();

    bool enabled = stateMachine->isActionWindowMode();

    ui->actionAccount->setEnabled(enabled);
    ui->actionEvents->setEnabled(enabled);
    ui->actionHODL->setEnabled( false);//  enabled);
    ui->actionAirdrop->setEnabled(enabled);
    ui->actionNode_status->setEnabled(enabled);
    ui->actionConnect_to_node->setEnabled(enabled);
    ui->actionContacts->setEnabled(enabled);
    ui->actionConfig->setEnabled(enabled);
    ui->actionListening_Status->setEnabled(enabled);
    ui->actionSend_coins->setEnabled(enabled);
    ui->actionRecieve_coins->setEnabled(enabled);
    ui->actionSend_recieve_offline->setEnabled(enabled);
    ui->actionTransactions->setEnabled(enabled);
    ui->actionOutputs->setEnabled(enabled);

    ui->actionAccount->setChecked( state == state::STATE::ACCOUNTS ) ;
    ui->actionEvents->setChecked( state == state::STATE::EVENTS ) ;
    ui->actionHODL->setChecked( state == state::STATE::HODL ) ;
    ui->actionAirdrop->setChecked( state == state::STATE::AIRDRDOP_MAIN ) ;
    ui->actionNode_status->setChecked( state == state::STATE::NODE_STATUS ) ;
    //ui->actionConnect_to_node;
    ui->actionContacts->setChecked( state == state::STATE::CONTACTS );
    ui->actionConfig->setChecked( state == state::STATE::WALLET_CONFIG );
    ui->actionListening_Status->setChecked( state == state::STATE::LISTENING );
    ui->actionSend_coins->setChecked( state == state::STATE::SEND_COINS );
    ui->actionRecieve_coins->setChecked( state == state::STATE::RECIEVE_COINS );
    ui->actionSend_recieve_offline->setChecked( state == state::STATE::FILE_TRANSACTIONS );
    ui->actionTransactions->setChecked( state == state::STATE::TRANSACTIONS );
    ui->actionOutputs->setChecked( state == state::STATE::OUTPUTS );

    if (!enabled)
        ui->mainToolBar->hide();
    else
        ui->mainToolBar->show();

}

void MainWindow::updateClock() {
    ui->currentTime->setText( QTime::currentTime().toString("hh:mm:ss") );
}

void MainWindow::on_actionVersion_triggered()
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
}


}




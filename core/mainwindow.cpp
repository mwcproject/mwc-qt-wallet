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
    ui->actionHODL->setEnabled(enabled);
    ui->actionAirdrop->setEnabled(enabled);
    ui->actionNode_status->setEnabled(enabled);
    ui->actionConnect_to_node->setEnabled(enabled);
    ui->actionContacts->setEnabled(enabled);
    ui->actionConfig->setEnabled(enabled);
    ui->actionListening_Status->setEnabled(enabled);
    ui->actionSend_coins->setEnabled(enabled);
    ui->actionSend_recieve_offline->setEnabled(enabled);
    ui->actionTransactions->setEnabled(enabled);
    ui->actionOutputs->setEnabled(enabled);

    ui->actionAccount->setChecked( state == state::STATE::ACCOUNTS ) ;
    ui->actionEvents->setChecked( state == state::STATE::EVENTS ) ;
    ui->actionHODL->setChecked( state == state::STATE::HODL ) ;
    //ui->actionAirdrop;
    //ui->actionNode_status;
    //ui->actionConnect_to_node;
    //ui->actionContacts;
    //ui->actionConfig;
    //ui->actionListening_Status;
    //ui->actionSend_coins;
    //ui->actionSend_recieve_offline;
    //ui->actionTransactions;
    //ui->actionOutputs;

}

void MainWindow::updateClock() {
    ui->currentTime->setText( QTime::currentTime().toString("hh:mm:ss") );
}

void MainWindow::on_actionVersion_triggered()
{
    VersionDialog * verDlg = new VersionDialog(this);
    verDlg->setModal(true);
    verDlg->exec();
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

}

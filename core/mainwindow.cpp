#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDebug>
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

}


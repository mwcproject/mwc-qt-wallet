// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDebug>
#include "../state/statemachine.h"
#include "util/widgetutils.h"
#include "util/stringutils.h"
#include "../control/messagebox.h"
#include "../dialogs/helpdlg.h"
#include "../core/Config.h"
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QScrollArea>
#include <QStyle>

namespace core {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef Q_OS_DARWIN
    Cocoa::changeTitleBarColor(winId(), 0x6F/255.0, 0.0, 0xD6/255.0 ); // Color #6f00d6
#endif

    ui->leftTb->hide();


    ui->statusBar->addPermanentWidget(ui->nodeStatusButton);
    ui->statusBar->addPermanentWidget(ui->listenerStatusButton);
    ui->statusBar->addPermanentWidget(ui->btnSpacerLabel1);
    ui->statusBar->addPermanentWidget(ui->helpButton);
    ui->statusBar->addPermanentWidget(ui->rightestSpacerLabel);

    if (config::isOnlineNode()) {
        ui->helpButton->hide();
        ui->listenerStatusButton->hide();
        ui->btnSpacerLabel1->hide();
    }

    if (config::isColdWallet()) {
        ui->listenerStatusButton->hide();
    }

    setStatusButtonState(ui->nodeStatusButton, STATUS::RED, "Waiting");
    setStatusButtonState(ui->listenerStatusButton, STATUS::RED, "Listeners");

    //ui->statusBar->showMessage("Can show any message here", 2000);

    // Want to delete children when they close
    setAttribute( Qt::WA_DeleteOnClose, true );

    // Update size by max screen size
    QSize wndSize = frameSize();
    QSize newSize = wndSize;
    QRect screenRc = QDesktopWidget().availableGeometry(this);

    const int DX = 10;
    const int DY = 30;

    if (newSize.width() > screenRc.width() - DX )
        newSize.setWidth( screenRc.width() - DX );

    if (newSize.height() > screenRc.height() - DY )
        newSize.setHeight( screenRc.height() - DY );

    if ( newSize != wndSize ) {
        QSize newSz = size() - ( wndSize - newSize );

        setGeometry( (screenRc.width() - newSize.width())/2,
                     (screenRc.height() - newSize.height())/2,
                     newSz.width(), newSz.height() );
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewNotificationMessage(notify::MESSAGE_LEVEL level, QString message) {

    using namespace wallet;

    QString prefix;
    int timeout = 3000;
    switch(level) {
        case notify::MESSAGE_LEVEL::FATAL_ERROR:
            prefix = "Error: ";
            timeout = 7000;
            break;
        case notify::MESSAGE_LEVEL::CRITICAL:
            prefix = "Critical: ";
            timeout = 7000;
            break;
        case notify::MESSAGE_LEVEL::WARNING:
            prefix = "Warning: ";
            timeout = 7000;
            break;
        case notify::MESSAGE_LEVEL::INFO:
            prefix = "Info: ";
            timeout = 4000;
            break;
        case notify::MESSAGE_LEVEL::DEBUG:
            prefix = "Debug: ";
            timeout = 2000;
            break;
    }

    ui->statusBar->showMessage( prefix + message, (int)(timeout * config::getTimeoutMultiplier()) );
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

    QObject::connect( notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
                     this, &MainWindow::onNewNotificationMessage, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onConfigUpdate,
                     this, &MainWindow::onConfigUpdate, Qt::QueuedConnection);


    QObject::connect(wallet, &wallet::Wallet::onMwcMqListenerStatus,
                     this, &MainWindow::updateListenerStatus, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onKeybaseListenerStatus,
                     this, &MainWindow::updateListenerStatus, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onNodeStatus,
                     this, &MainWindow::updateNodeStatus, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onUpdateSyncProgress,
                     this, &MainWindow::onUpdateSyncProgress, Qt::QueuedConnection);

    updateListenerBtn();
    updateNetworkName();
}

QWidget * MainWindow::getMainWindow() {
    return ui->mainWindowFrame;
}

void MainWindow::updateActionStates(state::STATE actionState) {

    bool isLeftBarVisible = (actionState >= state::STATE::ACCOUNTS && actionState != state::STATE::RESYNC );

    updateLeftBar( isLeftBarVisible );

    if (isLeftBarVisible) {
        ui->leftTb->updateButtonsState(actionState);
    }

//    bool enabled = stateMachine->isActionWindowMode();

}

void core::MainWindow::on_listenerStatusButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::LISTENING );
}

void core::MainWindow::on_nodeStatusButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::NODE_INFO );
}

void MainWindow::on_helpButton_clicked()
{
    state::State * state = stateMachine->getCurrentStateObj();
    QString docName = "";
    if (state != nullptr) {
        docName = state->getHelpDocName();
    }

    if ( docName.isEmpty() ) {
        docName = "default.html";
    }

    dlg::HelpDlg helpDlg(this, docName);
    helpDlg.exec();
}

void MainWindow::updateListenerStatus(bool online) {
    Q_UNUSED(online)
    updateListenerBtn();
}

// Node info
void MainWindow::updateNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg)
    Q_UNUSED(totalDifficulty)
    if ( !online ) {
        setStatusButtonState( ui->nodeStatusButton, STATUS::RED, "" );
    }
    else if (connections==0 || nodeHeight==0 || (peerHeight>0 && peerHeight-nodeHeight>5) ) {
        setStatusButtonState( ui->nodeStatusButton, STATUS::YELLOW, "" );
    }
    else {
        setStatusButtonState( ui->nodeStatusButton, STATUS::GREEN, "" );
    }
}

void MainWindow::onUpdateSyncProgress(double progressPercent) {
    onNewNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                             "Wallet state update, " + util::trimStrAsDouble( QString::number(progressPercent), 4 ) + "% complete"  );
}


void MainWindow::onConfigUpdate() {
    updateNetworkName();
}


void MainWindow::updateListenerBtn() {
    QPair<bool,bool> listStatus = wallet->getListenerStatus();
    qDebug() << "updateListenerBtn: " << listStatus;

    bool listening = listStatus.first | listStatus.second;
    QString listenerNames;
    if (listStatus.first)
        listenerNames +=  QString("MWC MQ") + (config::getUseMwcMqS() ? "S" : "");

    if (listStatus.second) {
        if (!listenerNames.isEmpty())
            listenerNames += ", ";
        listenerNames += "Keybase";
    }

    setStatusButtonState( ui->listenerStatusButton,
                          listening ? STATUS::GREEN : STATUS::RED,
                          listening ? listenerNames : "Listeners");
    ui->listenerStatusButton->setToolTip(listening ? "You are listening. Click here to view listener status"
                                          : "You are not listening. Click here to view listener status");
}


void MainWindow::updateNetworkName() {
    setStatusButtonState( ui->nodeStatusButton, STATUS::IGNORE, wallet->getWalletConfig().getNetwork() );
}

void MainWindow::setStatusButtonState(  QPushButton * btn, STATUS status, QString text ) {
    switch (status) {
        case STATUS::GREEN:
            btn->setIcon( QIcon( QPixmap( ":/img/CircGreen@2x.svg" )));
            break;
        case STATUS::RED:
            btn->setIcon( QIcon( QPixmap( ":/img/CircRed@2x.svg" )));
            break;
        case STATUS::YELLOW:
            btn->setIcon( QIcon( QPixmap( ":/img/CircYellow@2x.svg" )));
            break;
        default: // Ingnore suppose to be here
            break;
    }

    if (!text.isEmpty())
        btn->setText(" " + text + " ");
}

}


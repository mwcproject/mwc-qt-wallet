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

#include "statuswnd.h"
#include "ui_statuswnd.h"
#include "../core/global.h"
#include "../control_desktop/messagebox.h"
#include <QApplication>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QScreen>
#include <QStatusBar>
#include <QTimer>
#include <QFrame>
#include <QDebug>

namespace core {

// StatusWnd objects are controlled by MainWindow so that they can always be displayed
// on top of whatever windows might be visible at the time. If the MainWindow is destroyed,
// all StatusWnd objects will also be destroyed.
StatusWnd::StatusWnd(MainWindow* _mainWindow, int position, bool _mainWindowDisplay) :
    QWidget(_mainWindow),
    ui(new Ui::StatusWnd),
    mainWindow(_mainWindow),
    statusWindowNumber(position),
    mainWindowDisplay(_mainWindowDisplay)
{
    flags = Qt::Window | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint;
    if (mainWindowDisplay) {
        this->setParent(mainWindow);
        // note on flags, Qt::Tool seems to pause when wallet loses focus
        // Qt::SubWindow seems to keep running and stays with wallet and
        // doesn't appear in other screen where you are working
        // so can keep wallet up on one screen and work on another while
        // keeping an eye on the wallet notifications
        flags |= Qt::SubWindow;  // should always be kept on top of parent

        // settings which don't display when WindowStaysOnTopHint turned off and focus somewhere else
        //flags |= Qt::Tool;
        // setting which displays on top even when WindowStaysOnTopHint turned off
        //flags |= Qt::ToolTip;
        //flags |= Qt::Popup;
        //flags |= Qt::Widget;
        //flags |= Qt::Dialog;
        //flags |= Qt::Window;
    }
    else {
        // Qt::Dialog will end up being displayed on whichever screen you
        // are currently working on. So we only use it for the notifications
        // when the wallet is minimized or iconified
        flags |= Qt::Dialog;
    }
    QWidget::setWindowFlags(flags);

    // we hook up to the main application here as it can't be done in StatusWndMgr as it's
    // saved as a global too late
    mwcApp = mwc::getApplication();
    if (mwcApp) {
        QObject::connect(mwcApp, &QGuiApplication::applicationStateChanged, this, &StatusWnd::onApplicationStateChange, Qt::QueuedConnection);
    }

    // if we don't initialize ourself with the main window
    // we end up with crashes in the other windows off of the main
    // window (e.g. PanelBaseWnd, NavWnd). Not sure why, but make
    // sure to initialize with the main window as our parent.

    ui->setupUi(this);

    displayTimer = new QTimer(this);
    fadeInAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    fadeOutAnimation = new QPropertyAnimation(this, "windowOpacity", this);

    // set up animation properties
    // set the start and end values so the status will fade in
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    fadeInAnimation->setDuration(fadeInTime);
    fadeInAnimation->setEasingCurve(QEasingCurve::InQuad);
    // reverse the start and end values so the status will fade out
    fadeOutAnimation->setStartValue(1.0);
    fadeOutAnimation->setEndValue(0.0);
    fadeOutAnimation->setDuration(fadeOutTime);
    fadeOutAnimation->setEasingCurve(QEasingCurve::Linear);

    QObject::connect(displayTimer, &QTimer::timeout, this, &StatusWnd::fadeOut);
    QObject::connect(fadeInAnimation, &QPropertyAnimation::finished, this, &StatusWnd::startTimer);
    QObject::connect(fadeOutAnimation, &QPropertyAnimation::finished, this, &StatusWnd::fadeDone);

    ui->statusMessage->setAttribute(Qt::WA_TranslucentBackground);

    if (mainWindowDisplay) {
        ui->statusMessage->setFocusPolicy(Qt::ClickFocus);
    }
    else {
        ui->statusMessage->setFocusPolicy(Qt::NoFocus);
    }

    this->hide();
}

StatusWnd::~StatusWnd() {
    delete ui;
    delete displayTimer;
    delete fadeInAnimation;
    delete fadeOutAnimation;
}

void StatusWnd::displayMessage(QString message, int position) {
    hide();
    statusMessage = message;
    findStatusSummary();
    ui->statusMessage->setText(statusSummary);
    display(position);
    fadeIn();
}

void StatusWnd::findStatusSummary() {
    if (statusMessage.contains("MWCs sent successfully")) {
        // some message contain what we'd like to display as a status
        // embedded deep inside the message
        statusSummary = "MWCs sent successfully";
        extractSummary = false;
    }
    else if (statusMessage.contains("Congratulations")) {
        // special handling for receive MWC message as the amount
        // displayed contains a decimal point
        int endPos = statusMessage.indexOf("MWC") + 3;
        statusSummary = statusMessage.left(endPos);
        clearSummary = true;
    }
    else if (statusMessage.length() > statusSummaryLimit) {
        int endPos = -1;
        // find the end of the first sentence in the message
        endPos = statusMessage.indexOf('!');
        if (endPos == -1) {
            endPos = statusMessage.indexOf('.');
        }
        if (endPos == -1 || endPos > statusSummaryLimit) {
            statusSummary = statusMessage.left(statusSummaryLimit) + "...";
            clearSummary = true;
            extractSummary = false;
        }
        else {
            statusSummary = statusMessage.left(endPos);
        }
    }
    else
        statusSummary = statusMessage;
}

void StatusWnd::startTimer() {
    displayTimer->start(displayTime);
}

void StatusWnd::fadeIn() {
    fadeInAnimation->start(QAbstractAnimation::KeepWhenStopped);
}

void StatusWnd::fadeOut() {
    displayTimer->stop();
    fadeOutAnimation->start(QAbstractAnimation::KeepWhenStopped);
}

void StatusWnd::fadeDone() {
    hide();
    mainWindow->statusDone(this);
}

void StatusWnd::stopDisplay() {
    hide();
    if (displayTimer->isActive()) {
        displayTimer->stop();
    }
    fadeInAnimation->stop();
    fadeOutAnimation->stop();
    statusMessage.clear();
    statusSummary.clear();
    statusWindowNumber = -1;
}

void StatusWnd::display(int position) {
    statusWindowNumber = position;
    if (mainWindowDisplay) {
        displayOnMainWindow(statusWindowNumber);
    }
    else {
        // we are displaying the number of pending status messages
        displayOnMainScreen(statusWindowNumber);
    }
    this->show();
}

void StatusWnd::displayOnMainWindow(int windowPosition) {
    // we want to position the status window in the bottom right corner of the main window
    QPoint mwnPos = mainWindow->pos();
    QSize  swnSize = this->frameSize();
    QSize  mwnSize = mainWindow->size();
    const QStatusBar* sbar = mainWindow->statusBar();
    int sbarHeight = sbar->height() + 23; // height is always 27 as configured in .ui file, 23 is padding
    if (mainWindow->isFullScreen()) {
        sbarHeight = sbar->height() + (yScaleFactor * 23);
    }

    // position window relative to the right hand edge of the main window
    // and above the status bar
    int newX = mwnPos.x() + mwnSize.width() - swnSize.width() - 4;
    int newY = mwnPos.y() + mwnSize.height() - sbarHeight - ((swnSize.height() + 5) * windowPosition);
    move(newX, newY);
}

void StatusWnd::displayOnMainScreen(int windowPosition) {
    QSize swnSize = this->frameSize();
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();
    int newX = width - swnSize.width() - 40;
    int newY = height - swnSize.height() - (swnSize.height() * windowPosition);
    move (newX, newY);
}

void StatusWnd::on_statusMessage_clicked() {
    if (!mainWindowDisplay) {
        // hide this window
        mainWindow->hideWindow(this);
        // restore the main window
        mainWindow->restore();
        return;
    }
    QString messageTitle = "";
    QString messageText = statusMessage;
    if (statusSummary != statusMessage) {
        if (!clearSummary) {
            messageTitle = statusSummary;
        }
        if (extractSummary) {
            int startPos = statusMessage.indexOf('!');
            if (startPos == -1) {
                startPos = statusMessage.indexOf('.');
            }
            if (startPos != -1) {
                messageText = statusMessage.remove(0, startPos+2);  // include removal of punctuation plus space
            }
        }
    }
    control::MessageBox::messageText(this, messageTitle, messageText);
}

void StatusWnd::onApplicationStateChange(Qt::ApplicationState state) {
    // for status messages displayed in the wallet app, don't display the notification windows
    // on top if the wallet is not the active application
    if (mainWindowDisplay && state != currentState) {
        currentState = state;
        hide();
        if (state == Qt::ApplicationActive) {
            qDebug() << "Turning on WindowStaysOnTopHint";
            flags |= Qt::WindowStaysOnTopHint;
            QWidget::setWindowFlags(flags);
        }
        else {
            qDebug() << "Turning off WindowStaysOnTopHint";
            flags &= ~Qt::WindowStaysOnTopHint;
            QWidget::setWindowFlags(flags);
        }
        display(statusWindowNumber);
    }
}

}



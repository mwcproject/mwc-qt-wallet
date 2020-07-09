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
#include "../control_desktop/messagebox.h"
#include <QApplication>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QScreen>
#include <QSharedPointer>
#include <QStatusBar>
#include <QTimer>
#include <QFrame>
#include <QDebug>

namespace core {

// StatusWnd objects are controlled by MainWindow so that they can always be displayed
// on top of whatever windows might be visible at the time. If the MainWindow is destroyed,
// all StatusWnd objects will also be destroyed. If the MainWindow is iconified, StatusWnd
// objects will still appear on whichever screen the dock appears on.
StatusWnd::StatusWnd(MainWindow* _mainWindow, QString _statusMessage, int windowPosition, bool _clickable) :
    QWidget(_mainWindow),
    ui(new Ui::StatusWnd),
    mainWindow(_mainWindow),
    statusMessage(_statusMessage),
    clickable(_clickable)
{
    Qt::WindowFlags flags = Qt::Window;
    if (clickable) {
        // note on flags, Qt::Tool seems to pause when wallet loses focus
        // Qt::SubWindow seems to keep running and stays with wallet and
        // doesn't appear in other screen where you are working
        // so can keep wallet up on one screen and work on another while
        // keeping an eye on the wallet notifications
        flags |= Qt::SubWindow;  // should always be kept on top of parent
        flags |= Qt::WindowStaysOnTopHint;
    }
    else {
        // Qt::Dialog will end up being displayed on whichever screen you
        // are currently working on. So we only use it for the notifications
        // when the wallet is minimized or iconified
        flags |= Qt::Dialog;
        flags |= Qt::WindowStaysOnTopHint;
    }
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::NoDropShadowWindowHint;
    QWidget::setWindowFlags(flags);

    // if we don't initialize ourself with the main window
    // we end up with crashes in the other windows off of the main
    // window (e.g. PanelBaseWnd, NavWnd). Not sure why, but make
    // sure to initialize with the main window as our parent.

    ui->setupUi(this);
    if (!clickable)
        ui->statusMessage->setFocusPolicy(Qt::NoFocus);
    findStatusSummary();
    ui->statusMessage->setText(statusSummary);
    ui->statusMessage->setAttribute(Qt::WA_TranslucentBackground);

    display(windowPosition);
    this->show();
    fadeIn();
}

StatusWnd::~StatusWnd() {
    delete ui;
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
    QTimer::singleShot(displayTime, this, &StatusWnd::fadeOut);
}

void StatusWnd::fadeIn() {
    QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
    connect(animation, &QPropertyAnimation::finished, this, &StatusWnd::startTimer);

    // reversing the start and end values will cause the status to fade out vs fade in
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setDuration(fadeInTime);
    animation->setEasingCurve(QEasingCurve::InQuad);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void StatusWnd::fadeOut() {
    QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
    connect(animation, &QPropertyAnimation::finished, this, &StatusWnd::fadeDone);

    // reversing the start and end values will cause the status to fade in vs fade out
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->setDuration(fadeOutTime);
    animation->setEasingCurve(QEasingCurve::Linear);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void StatusWnd::fadeDone() {
    hide();
    mainWindow->statusDone(QSharedPointer<StatusWnd>(this));
}

void StatusWnd::display(int windowPosition) {
    if (!clickable) {
        // we are displaying the number of pending status messages
        displayOnMainScreen(windowPosition);
    }
    else {
        displayOnMainWindow(windowPosition);
    }
}

void StatusWnd::displayOnMainWindow(int windowPosition) {
    // we want to position the status window in the bottom right corner of the main window
    QPoint mwnPos = mainWindow->pos();
    QSize  swnSize = this->frameSize();
    QSize  mwnSize = mainWindow->size();
    const QStatusBar* sbar = mainWindow->statusBar();
    QPoint sbarPos = sbar->pos();
    int sbarHeight = sbar->height() + 23; // height is always 27 as configured in .ui file, 23 is padding
    if (mainWindow->isFullScreen()) {
        sbarHeight = sbar->height() + (yScaleFactor * 23);
    }
    qDebug() << "mainWindow pos - x: " << QString::number(mwnPos.x()) << " y: " << QString::number(mwnPos.y());
    qDebug() << "mainWindow size - width: " << QString::number(mwnSize.width()) << " height: " << QString::number(mwnSize.height());
    qDebug() << "statusbar pos - x: " << QString::number(sbarPos.x()) << " y: " << QString::number(sbarPos.y());
    qDebug() << "statusbar height: " << QString::number(sbarHeight) << "statusSummary: " << statusSummary;

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
    if (!clickable) {
        // hide this window
        mainWindow->statusHide(QSharedPointer<StatusWnd>(this));
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

}



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

#include "../bridge/config_b.h"

#include "../core/global.h"
#include "../core_desktop/mainwindow.h"
#include "../core_desktop/statuswnd.h"
#include "../core_desktop/statuswndmgr.h"

#include "../state/state.h"
#include "../wallet/wallet.h"

#include <QWidget>
#include <QEvent>
//#include <QDebug>

namespace core {

// Class to manage StatusWnd objects for the MainWindow.
StatusWndMgr::StatusWndMgr(core::MainWindow* mainwindow) :
    mainWindow(mainwindow)
{
}

StatusWndMgr::~StatusWndMgr() {
    removeWindows();
}

void StatusWndMgr::initWindows() {
    // create windows for notifications once and reuse as needed
    for (int i=0; i<maxStatusDisplay; i++) {
        StatusWnd* swnd = new StatusWnd(mainWindow);
        statusWindowList.append(swnd);
    }

    // window 0 displays the number of pending messages when the wallet is iconified
    // displays on screen above task bar
    StatusWnd* pwnd = new StatusWnd(mainWindow, false);
    pendingWindowList.append(pwnd);
    // window 1 displays the number of pending messages when the wallet is locked
    // displays on wallet
    pwnd = new StatusWnd(mainWindow, true);
    pwnd->disableClickable();
    pendingWindowList.append(pwnd);
}

void StatusWndMgr::removeWindows() {
    for (int i=0; i<maxStatusDisplay; i++) {
        StatusWnd* swnd = statusWindowList.takeFirst();
        swnd->stopDisplay();
        delete swnd;
    }

    for (int j=0; j<numPendingMsgWindows; j++) {
        StatusWnd* pwnd = pendingWindowList.takeFirst();
        pwnd->stopDisplay();
        delete pwnd;
    }
}

void StatusWndMgr::restore() {
    if (previouslyMinimized) {
        previouslyMinimized = false;
        prevPendingMsgCount = 0;

        // restore the main window from being minimized
        mainWindow->setWindowState(Qt::WindowActive);

        displayPendingStatusMessages();
    }
}

bool StatusWndMgr::filterOutMessage(QString message) {
    bool filterOut = false;
    if (message.contains("Wallet state update, ")) {
        // only display messages that would also go into the event log
        filterOut = true;
    }
    else if (mainWindow->config->isColdWallet()) {
        if (message.contains("listener") || message.contains("sync")) {
            // the cold wallet is expected to be offline
            // don't display messages about the listeners
            filterOut = true;
        }
    }
    return filterOut;
}

void StatusWndMgr::handleStatusMessage(QString prefix, QString message) {
    // only display messages that would also go into the event log
    if (filterOutMessage(message))
        return;

    // perform one-time initialization of the status windows
    // can't do this in CTOR as entire system isn't set up yet and
    // StatusWnd needs access to wallet applicatian (QApplication)
    if (statusWindowList.size() == 0) {
        initWindows();
    }

    // skip any duplicate messages
    if (pendingStatusMessages.size() > 0 && pendingStatusMessages.last() == message)
        return;

    // only append the message if there is room in the queue
    // the call to displayPendingStatusMessages() will then check
    // if we can take and display any more messages from the queue
    if (pendingStatusMessages.size() < pendingMsgLimit) {
        pendingStatusMessages.append(prefix + message);
    }
    displayPendingStatusMessages();
}

void StatusWndMgr::displayPendingStatusMessages() {
    if (mwc::isWalletLocked() || mainWindow->isMinimized() || !loginOk) {
        // display pending messages if the wallet is locked or minimized
        // or the wallet is no longer in the locked state but the login did not succeed
        displayNumberPendingMessages();
    }
    else {
        prevPendingMsgCount = 0;  // used when the wallet is locked or minimized, reset when not

        // display pending status messages as room allows
        while (!pendingStatusMessages.isEmpty() && visibleMsgCount < maxStatusDisplay) {
            QString statusMsg = pendingStatusMessages.takeFirst();
            if (!statusMsg.isEmpty()) {
                StatusWnd* swnd = statusWindowList.value(visibleMsgCount);
                swnd->displayMessage(statusMsg, visibleMsgCount);
                visibleMsgCount++;
            }
        }
    }
}

void StatusWndMgr::displayNumberPendingMessages() {
    int atLimitCounter = 0;

    // Hide any normal status messages
    if (visibleMsgCount > 0) {
        hideStatusWindows();
        // ensure counter is reset as we were previously displaying normal notifications
        atLimitCounter = 0;
    }
    // Hide any num pending status windows
    hidePendingWindows();

    int pendingMsgCount = pendingStatusMessages.size();
    if (pendingMsgCount == 0) {
        atLimitCounter = 0;
        return;
    }

    // if we are at the limit, only display the at limit message once every time the
    // limit counter is zero
    // otherwise the mining wallet is constantly displaying the limit notfications
    if (pendingMsgCount == pendingMsgLimit && prevPendingMsgCount == pendingMsgCount) {
        // we've displayed the at limit message previously
        if (atLimitCounter == pendingMsgLimit) {
            // allow another at limit message to be displayed
            atLimitCounter = 0;
        }
        else {
            atLimitCounter++;
        }
    }
    else {
        atLimitCounter = 0;
    }

    //qDebug() << "atLimitCounter: " << QString::number(atLimitCounter);
    if (atLimitCounter == 0 && prevPendingMsgCount != pendingMsgCount) {
        QString statusMsg = "Notifications Waiting To Be Read: " + QString::number(pendingMsgCount);
        if (pendingMsgCount >= pendingMsgLimit) {
            statusMsg = "Notifications Waiting To Be Read At Limit: " + QString::number(pendingMsgLimit);
        }

        // Sometimes messages come in quickly, so usually the last message count is only displayed
        StatusWnd* pwnd = nullptr;
        if (mainWindow->isMinimized()) {
            pwnd = pendingWindowList.value(pendingMsgScreenWindow);
        }
        else {
            pwnd = pendingWindowList.value(pendingMsgWalletWindow);
        }
        pwnd->displayMessage(statusMsg, 0);
        prevPendingMsgCount = pendingMsgCount;
    }
}

void StatusWndMgr::hideStatusWindows() {
    for (int i=0; i<statusWindowList.size(); i++) {
        StatusWnd* swnd = statusWindowList.value(i);
        swnd->stopDisplay();
    }
    visibleMsgCount = 0;
}

void StatusWndMgr::hidePendingWindows() {
    for (int i=0; i<numPendingMsgWindows; i++) {
        StatusWnd* pwnd = pendingWindowList.value(i);
        pwnd->stopDisplay();
    }
}

void StatusWndMgr::hideWindow(StatusWnd* swnd) {
    swnd->stopDisplay();
    if (visibleMsgCount > 0) {
        // reposition any visible status windows
        int idx = statusWindowList.indexOf(swnd);
        statusWindowList.removeAt(idx);

        int newPosition = 0;
        // reposition all of the remaining status messages
        for (int i=0; i<statusWindowList.size(); i++) {
            StatusWnd* rwnd = statusWindowList.value(i);
            if (rwnd->windowPosition() == -1) {
                continue;
            }
            rwnd->hide();
            rwnd->display(newPosition);
            newPosition++;
        }
        visibleMsgCount = newPosition;
        // append the status window to the back of the list
        statusWindowList.append(swnd);
    }
}

void StatusWndMgr::statusDone(StatusWnd* swnd) {
    // hide and reposition any statuses still visible
    hideWindow(swnd);
    // display any pending messages if there is room
    displayPendingStatusMessages();
}

void StatusWndMgr::redisplayStatusWindows() {
    // redisplay all of the visible messages
    if (visibleMsgCount > 0) {
        for (int i=0; i<visibleMsgCount; i++) {
            StatusWnd* swnd = statusWindowList.value(i);
            swnd->display(i);
        }
    }
}

void StatusWndMgr::moveEvent(QMoveEvent* event) {
    Q_UNUSED(event);

    if (visibleMsgCount > 0) {
        // reposition all of the visible status messages
        redisplayStatusWindows();
    }
    else if (pendingStatusMessages.size() > 0) {
        displayPendingStatusMessages();
    }
}

void StatusWndMgr::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);

    // redisplay all of the visible messages
    redisplayStatusWindows();
}

bool StatusWndMgr::event(QEvent* event) {
    bool eventConsumed = false;
    if (event->type() == QEvent::WindowStateChange) {
        //qDebug() << "QEvent::WindowStateChange";
        if (mainWindow->isMinimized()) {
            eventConsumed = true;
            previouslyMinimized = true;
            hideStatusWindows();
            displayNumberPendingMessages();
        }
        else if (mainWindow->isVisible() && previouslyMinimized) {
            // we just changed from being minimized to visible
            // hide any previously shown windows
            hidePendingWindows();
            // reset the main window and any state
            restore();
            eventConsumed = true;
        }
        else if ((mainWindow->isFullScreen() && !previouslyFullScreen) ||
                 (!mainWindow->isFullScreen() && previouslyFullScreen))
        {
            previouslyFullScreen = mainWindow->isFullScreen();
            // redisplay all of the status windows
            redisplayStatusWindows();
        }
    }
    return eventConsumed;
}

void StatusWndMgr::onLoginResult(bool ok) {
    loginOk = ok;
    visibleMsgCount = 0;
}

void StatusWndMgr::onApplicationStateChange(Qt::ApplicationState state) {
    // for status messages displayed in the wallet app, don't display the notification windows
    // on top if the wallet is not the active application
    if (state != currentState) {
        currentState = state;
        bool windowOnTop = false;
        if (state == Qt::ApplicationActive) {
            windowOnTop = true;
        }
        // inform status windows of new state
        for (int i=0; i<statusWindowList.size(); i++) {
            StatusWnd* swnd = statusWindowList.value(i);
            swnd->checkWindowFlags(windowOnTop);
        }
        // inform num messages pending window for wallet
        StatusWnd* pwnd = pendingWindowList.value(pendingMsgWalletWindow);
        pwnd->checkWindowFlags(windowOnTop);
        if (visibleMsgCount == 0) {
            displayNumberPendingMessages();
        }
    }
}


}



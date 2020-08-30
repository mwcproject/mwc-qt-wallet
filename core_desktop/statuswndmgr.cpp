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

#include "../core/global.h"
#include "../core_desktop/mainwindow.h"
#include "../core_desktop/statuswnd.h"
#include "../core_desktop/statuswndmgr.h"
#include <QWidget>
#include <QEvent>
#include <QDebug>

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
        StatusWnd* swnd = new StatusWnd(mainWindow, i);
        statusWindowList.append(swnd);
    }

    // window 0 displays the number of pending messages when the wallet is iconified
    // displays on screen above task bar
    StatusWnd* pwnd = new StatusWnd(mainWindow, pendingMsgScreenWindow, false);
    pendingWindowList.append(pwnd);
    // window 1 displays the number of pending messages when the wallet is locked
    // displays on wallet
    pwnd = new StatusWnd(mainWindow, pendingMsgWalletWindow, true);
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
        mainWindow->setWindowState(mainWindow->windowState() & !Qt::WindowMinimized | Qt::WindowActive);

        displayPendingStatusMessages();
    }
}

void StatusWndMgr::handleStatusMessage(QString prefix, QString message) {
    // only display messages that would also go into the event log
    if (message.contains("Wallet state update, "))
        return;

    // perform one-time initialization of the status windows
    // can't do this in CTOR as entire system isn't set up yet and
    // StatusWnd needs access to wallet applicatian (QApplication)
    if (statusWindowList.size() == 0) {
        initWindows();
    }

    // skip any duplicates
    if (pendingStatusMessages.size() > 0 && pendingStatusMessages.last() == message)
        return;
    pendingStatusMessages.append(prefix + message);
    // limit the messages stored for later display
    if (pendingStatusMessages.size() > pendingMsgLimit) {
        pendingStatusMessages.removeLast();
    }
    displayPendingStatusMessages();
}

void StatusWndMgr::displayPendingStatusMessages() {
    if (mwc::isWalletLocked() || mainWindow->isMinimized()) {
        // display pending messages if the wallet is locked or minimized
        displayNumberPendingMessages();
    }
    else {
        prevPendingMsgCount = 0;  // used when the wallet is locked or minimized, reset when not

        // display pending status messages as room allows
        while (!pendingStatusMessages.isEmpty() && visibleMsgCount < maxStatusDisplay) {
            QString statusMsg = pendingStatusMessages.takeFirst();
            StatusWnd* swnd = statusWindowList.value(visibleMsgCount);
            swnd->displayMessage(statusMsg, visibleMsgCount);
            visibleMsgCount++;
        }
    }
}

void StatusWndMgr::displayNumberPendingMessages() {
    if (!pendingStatusMessages.isEmpty()) {
        int pendingMsgCount = pendingStatusMessages.size();
        if (pendingMsgCount != prevPendingMsgCount || pendingMsgCount >= pendingMsgLimit) {
            QString statusMsg = "Notifications Waiting To Be Read: " + QString::number(pendingMsgCount);
            if (pendingMsgCount >= pendingMsgLimit) {
                statusMsg = "Notifications Waiting To Be Read At Limit: " + QString::number(pendingMsgLimit);
            }
            // Hide any normal status messages
            if (visibleMsgCount > 0) {
                hideStatusWindows();
            }

            // Sometimes messages come in quickly, so usually the last message count is only displayed
            StatusWnd* pwnd = pendingWindowList.first();
            pwnd->stopDisplay();
            pwnd->displayMessage(statusMsg, 0);
            prevPendingMsgCount = pendingMsgCount;
        }
    }
}

void StatusWndMgr::hideStatusWindows() {
    for (int i=0; i<statusWindowList.size(); i++) {
        StatusWnd* swnd = statusWindowList.value(i);
        swnd->stopDisplay();
        swnd->resetWindowPosition();
    }
    visibleMsgCount = 0;
}

void StatusWndMgr::hidePendingWindow() {
    StatusWnd* pwnd = pendingWindowList.first();
    pwnd->stopDisplay();
}

void StatusWndMgr::hideWindow(StatusWnd* swnd) {
    swnd->stopDisplay();
    if (visibleMsgCount > 0) {
        // reposition any visible status windows
        int idx = statusWindowList.indexOf(swnd);
        statusWindowList.removeAt(idx);

        int newPosition = 0;
        // reposition all of the remain status messages
        for (int i=0; i<statusWindowList.size(); i++) {
            StatusWnd* rwnd = statusWindowList.value(i);
            if (rwnd->windowPosition() == -1) {
                continue;
            }
            qDebug() << "moving window[" << QString::number(i) << "] to position: " << QString::number(newPosition);
            rwnd->display(newPosition++);
        }
        visibleMsgCount = newPosition;
        // reset and append the status window to the back of the list
        swnd->resetWindowPosition();
        statusWindowList.append(swnd);
    }
}

void StatusWndMgr::statusDone(StatusWnd* swnd) {
    // hide and reposition any statuses still visible
    hideWindow(swnd);
    // display any pending messages if there is room
    displayPendingStatusMessages();
}

void StatusWndMgr::moveEvent(QMoveEvent* event) {
    Q_UNUSED(event);

    if (visibleMsgCount > 0) {
        for (int i=0; i<visibleMsgCount; i++) {
            // reposition all of the visible status messages
            StatusWnd* swnd = statusWindowList.value(i);
            swnd->display(i);
        }
    }
    else if (pendingStatusMessages.size() > 0) {
        displayPendingStatusMessages();
    }
}

void StatusWndMgr::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);

    // redisplay all of the visible messages
    if (visibleMsgCount > 0) {
        for (int i=0; i<visibleMsgCount; i++) {
            StatusWnd* swnd = statusWindowList.value(i);
            swnd->display(i);
        }
    }
}

bool StatusWndMgr::event(QEvent* event) {
    bool eventConsumed = false;
    if (event->type() == QEvent::WindowStateChange) {
        if (mainWindow->isMinimized()) {
            eventConsumed = true;
            previouslyMinimized = true;
            hideStatusWindows();
            displayNumberPendingMessages();
        }
        else if (mainWindow->isVisible() && previouslyMinimized) {
            // we just changed from being minimized to visible
            // hide any previously shown windows
            hideStatusWindows();
            // reset the main window and any state
            restore();
            eventConsumed = true;
        }
    }
    return eventConsumed;
}


}



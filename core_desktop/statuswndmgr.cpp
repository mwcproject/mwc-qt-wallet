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
#include <QDebug>
#include <QEvent>

namespace core {

// Class to manage StatusWnd objects for the MainWindow.
StatusWndMgr::StatusWndMgr(core::MainWindow* mainwindow) :
    mainWindow(mainwindow)
{
}

StatusWndMgr::~StatusWndMgr() {
    statusWindowList.clear();  // deletes any remaining smart pointers
    pendingStatusMessages.clear();
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
        while (!pendingStatusMessages.isEmpty() && statusWindowList.size() < maxStatusDisplay) {
            QString statusMsg = pendingStatusMessages.takeFirst();
            // because StatusWnd uses a timer, we always want to use deleteLater for object destruction
            // deleteLater won't let the object be destroyed until control returns to the event loop so we
            // don't have to worry about the timer firing and causing a crash because the object has already
            // been deleted
            QSharedPointer<StatusWnd> swnd = QSharedPointer<StatusWnd>(new StatusWnd(mainWindow, statusMsg, statusWindowList.size()), &StatusWnd::deleteLater);
            statusWindowList.append(swnd);
            swnd->show();
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
            // Hide any previous pending msg count messages
            // Sometimes messages come in quickly, so usually the last message count is only displayed
            if (statusWindowList.size() > 0) {
                statusHideAll();
            }
            QSharedPointer<StatusWnd> swnd = QSharedPointer<StatusWnd>(new StatusWnd(mainWindow, statusMsg, 0, false), &StatusWnd::deleteLater);
            statusWindowList.append(swnd);
            swnd->show();
            prevPendingMsgCount = pendingMsgCount;
        }
    }
}

void StatusWndMgr::statusHideAll() {
    while (statusWindowList.size() > 0) {
        QSharedPointer<StatusWnd> swnd = statusWindowList.last();
        swnd->hide();
        statusWindowList.removeLast();
    }
}

void StatusWndMgr::statusHide(const QSharedPointer<StatusWnd> _swnd) {
    _swnd->hide();
    int idx = statusWindowList.indexOf(_swnd);
    if (idx >= 0 && idx < statusWindowList.size()) {
        // because we are using shared pointers, we can just remove
        // the status window from our list, we don't have to explicitly
        // delete it
        statusWindowList.removeAt(idx);
    }
    for (int i = 0; i < statusWindowList.size(); i++) {
        // reposition all of the remaining status messages
        QSharedPointer<StatusWnd> swnd = statusWindowList.value(i);
        swnd->display(i);
    }
}

void StatusWndMgr::statusDone(const QSharedPointer<StatusWnd> _swnd) {
    // hide and reposition any statuses still visible
    statusHide(_swnd);
    // display any pending messages if there is room
    displayPendingStatusMessages();
}

void StatusWndMgr::moveEvent(QMoveEvent* event) {
    Q_UNUSED(event);

    if (statusWindowList.size() > 0) {
        for (int i = 0; i < statusWindowList.size(); i++) {
            // reposition all of the remaining status messages
            QSharedPointer<StatusWnd> statusWin = statusWindowList.value(i);
            statusWin->display(i);
        }
    }
    else if (pendingStatusMessages.size() > 0) {
        displayPendingStatusMessages();
    }
}

void StatusWndMgr::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);

    for (int i = 0; i < statusWindowList.size(); i++) {
        // reposition all of the remaining status messages
        QSharedPointer<StatusWnd> statusWin = statusWindowList.value(i);
        statusWin->display(i);
    }
}

bool StatusWndMgr::event(QEvent* event) {
    bool eventConsumed = false;
    if (event->type() == QEvent::WindowStateChange) {
        if (mainWindow->isMinimized()) {
            eventConsumed = true;
            previouslyMinimized = true;
            statusHideAll();
            displayNumberPendingMessages();
        }
        else if (mainWindow->isVisible() && previouslyMinimized) {
            // we just changed from being minimized to visible
            // hide any previously shown windows
            statusHideAll();
            // reset the main window and any state
            restore();
            eventConsumed = true;
        }
    }
    return eventConsumed;
}


}



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

#ifndef STATUSWNDMGR_H
#define STATUSWNDMGR_H

#include <QList>

namespace core {

class MainWindow;
class StatusWnd;

// Non-widget class used to control the status windows from the main window
class StatusWndMgr {
public:
    StatusWndMgr(core::MainWindow* mainwindow);
    ~StatusWndMgr();

    void restore();  // restore after being iconified

    void handleStatusMessage(QString prefix, QString message);
    void statusHide(const QSharedPointer<StatusWnd> _swnd);
    void statusDone(const QSharedPointer<StatusWnd> _swnd);
    void moveEvent(QMoveEvent* event);
    void resizeEvent(QResizeEvent* event);
    bool event(QEvent* event);

protected:
    void displayPendingStatusMessages();
    void displayNumberPendingMessages();
    void statusHideAll();

private:
    core::MainWindow*                mainWindow;

    bool                             previouslyMinimized = false;
    int                              maxStatusDisplay = 5;
    int                              prevPendingMsgCount = 0;
    int                              pendingMsgLimit = 25;
    QList<QSharedPointer<StatusWnd>> statusWindowList;
    QList<QString>                   pendingStatusMessages;
};

}

#endif // STATUSWNDMGR_H

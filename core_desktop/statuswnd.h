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

#ifndef STATUSWND_H
#define STATUSWND_H

#include "mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <QPropertyAnimation>

namespace Ui {
class StatusWnd;
}

namespace core {

class MainWindow;

// Window with navigation bar
class StatusWnd : public QWidget {
    Q_OBJECT
public:
    StatusWnd(core::MainWindow* mainWindow, bool mainWindowDisplay = true);
    ~StatusWnd();
    void disableClickable();
    void display(int position);
    void displayMessage(QString message, int position);
    void stopDisplay();
    int windowPosition() { return statusWindowNumber; }
    void checkWindowFlags(bool displayOnTop);

private slots:
    void on_statusMessage_clicked();

protected:
    void startTimer();
    void fadeIn();
    void fadeOut();
    void fadeDone();
    void findStatusSummary();
    void displayOnMainWindow(int windowPosition);
    void displayOnMainScreen(int windowPosition);

private:
    Ui::StatusWnd*        ui = nullptr;
    core::MainWindow*     mainWindow = nullptr;
    Qt::WindowFlags       flags;

    int                   statusWindowNumber = -1;
    bool                  mainWindowDisplay = true;  // display on wallet main window vs system main screen
    bool                  clickable = true;

    QString               statusMessage;
    QString               statusSummary;

    int                   statusSummaryLimit = 50;
    bool                  extractSummary     = true;
    bool                  clearSummary       = false;

    int                   displayTime = 6000;
    int                   fadeInTime  = 2000;
    int                   fadeOutTime = 1000;

    QTimer*               displayTimer;
    QPropertyAnimation*   fadeInAnimation;
    QPropertyAnimation*   fadeOutAnimation;

    // the main window status bar height is hard-coded to 27 in the .ui file
    // but some screens change the height of the status bar
    // to consistently display the status windows at the same location relative
    // to the main window, we use a constant status bar height of 27
    int                   mwnStatusBarHeight = 27;
    int                   yScaleFactor = 2;
};

}

#endif // STATUSWND_H

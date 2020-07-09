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
#include <QWidget>
#include <QWidget>

namespace Ui {
class StatusWnd;
}

namespace core {

class MainWindow;

// Window with navigation bar
class StatusWnd : public QWidget {
    Q_OBJECT
public:
    StatusWnd(core::MainWindow* mainWindow, QString statusMessage, int windowPosition, bool clickable = true);
    ~StatusWnd();
    void display(int windowPosition);

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

    int                   statusSummaryLimit = 50;
    bool                  extractSummary     = true;
    bool                  clearSummary       = false;
    QString               statusSummary;
    QString               statusMessage;

    bool                  clickable = true;
    int                   fadeInTime  = 2000;
    int                   displayTime = 6000;
    int                   fadeOutTime = 2000;
    int                   yScaleFactor = 2;
};

}

#endif // STATUSWND_H

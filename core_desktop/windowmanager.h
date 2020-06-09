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

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include "mainwindow.h"
#include <QLayout>

namespace wallet {
    class Wallet;
};

namespace bridge {
class Wallet;
class config;
}

namespace core {

// WIndows menager is responsible for connection between UI and the data
class WindowManager : public QObject
{
    Q_OBJECT
public:
    WindowManager( core::MainWindow * mainWnd, QWidget * pageHostWnd );

    // Show new window and return it
    QWidget * switchToWindowEx( const QString & pageName, QWidget * newWindow );
    // Parent for windows it can show.
    QWidget * getInWndParent() const;

private:
    QString buildWalletTitle(const QString & pageName);

private:
    core::MainWindow * mainWnd;     // App main wnd
    QWidget * pageHostWnd;          // Parent windows for pages.
    QWidget * currentWnd = nullptr; // Current active page. Single page can be active at a time
    bridge::Wallet * wallet = nullptr;
    bridge::Config * config = nullptr;
};

}

#endif // WINDOWMANAGER_H

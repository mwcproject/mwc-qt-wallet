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

#include "global.h"
#include <QApplication>
#include <QMainWindow>
#include "../core/Config.h"

namespace mwc {

static QApplication * mwcApp = nullptr;
static QMainWindow * mwcMainWnd = nullptr;

QString get_APP_NAME() {
    if (config::isOnlineWallet())
        return "mwc wallet";
    else if (config::isOnlineNode())
        return "mwc node";
    else if (config::isColdWallet())
        return "mwc cold wallet";
    else
        return "Unknown mode";
}

void setApplication(QApplication * app, QMainWindow * mainWindow) {
    mwcApp = app;
    mwcMainWnd = mainWindow;
}

static bool nonClosed = true;

bool isAppNonClosed() {return nonClosed;}

void closeApplication() {
    if (mwcApp == nullptr || mwcMainWnd==nullptr) {
        nonClosed = false;
        return; // posible if error detected during the start of the app. Nothing is there yet
    }

    Q_ASSERT(mwcApp);
    Q_ASSERT(mwcMainWnd);

    if (nonClosed) {

        nonClosed = false;

        // Async call is the only way to close App nicely !!!!
        // Alternatively we can call app::quit slot
        QMetaObject::invokeMethod(mwcMainWnd, "close", Qt::QueuedConnection);
    }
}

}

// Copyright 2020 The MWC Developers
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

#include "WalletApp.h"
#include <QDebug>
#include "../state/statemachine.h"

namespace core {

static bool appCreated = false;
static bool appInitialized = false;
static bool isAppExiting = false;

WalletApp::WalletApp(int &argc, char **argv) :
    QApplication(argc, argv)
{
    appCreated = true;
}

bool WalletApp::notify(QObject *receiver, QEvent *event) {
    bool ret = QApplication::notify(receiver, event);
    if (event) {
        if (event->type()==QEvent::KeyPress || event->type()==QEvent::MouseButtonPress) {
            //qDebug() << "Detecting user activity, resetting the locking timer";
            if (state::getStateMachine()!= nullptr)
                state::getStateMachine()->resetLogoutLimit(false);
        }
    }
    return ret;
}

// app is done with init process, basic objects should be good to go
void WalletApp::reportAppAsInitialized() {
    appInitialized = true;
}

bool WalletApp::isAppCreated() {
    return appCreated;
}

bool WalletApp::isAppInitialized() {
    return appInitialized;
}

bool WalletApp::isExiting() {
    return isAppExiting;
}

void WalletApp::startExiting() {
    isAppExiting = true;
}


}

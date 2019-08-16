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

#include "windowmanager.h"
#include "../wallet/wallet.h"
#include <QDebug>
#include <QApplication>
#include "../windows/c_newwallet_w.h"
#include "../windows/c_newseed_w.h"
#include "../windows/c_enterseed.h"

namespace core {

WindowManager::WindowManager(QWidget  * mainWnd ) :
    mainWindow(mainWnd)
{
    Q_ASSERT(mainWindow);
}

QWidget * WindowManager::getInWndParent() const {
    return mainWindow;
}


QWidget * WindowManager::switchToWindowEx( QWidget * newWindow ) {
    if (currentWnd==newWindow)
        return newWindow;

    if (currentWnd!=nullptr) {
        currentWnd->close();
        currentWnd = nullptr;
    }
    if (newWindow==nullptr)
        return newWindow;

    currentWnd = newWindow;
    currentWnd->setAttribute( Qt::WA_DeleteOnClose );
    mainWindow->layout()->addWidget(currentWnd);
    currentWnd->show();
    return newWindow;
}

}


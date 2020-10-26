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
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "paneltitle.h"

using namespace bridge;

namespace core {

WindowManager::WindowManager(core::MainWindow * _mainWnd, QWidget * _pageHostWnd) :
    mainWnd(_mainWnd),
    pageHostWnd(_pageHostWnd)
{
    Q_ASSERT(mainWnd);
    Q_ASSERT(pageHostWnd);

    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
}

QWidget * WindowManager::getInWndParent() const {
    return pageHostWnd;
}


QWidget * WindowManager::switchToWindowEx( const QString & pageName, QWidget * newWindow ) {
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
    pageHostWnd->layout()->addWidget(currentWnd);
    currentWnd->show();

    pageHostWnd->repaint();
    currentWnd->repaint();

    mainWnd->setWindowTitle(buildWalletTitle(pageName));
    setCurrentTitle(pageName);

    return newWindow;
}

QString WindowManager::buildWalletTitle(const QString & pageName) {
    Q_UNUSED(pageName)

    QString buildNumber = config->getBuildVersion();
    QVector<QString> ppin = config->getCurrentWalletInstance();

    QString title = config->get_APP_NAME() + " v" + buildNumber + " [" + ppin[2] + "]";

    /* we are printing page name on the page now...
    if (!pageName.isEmpty()) {
        title += " - " + pageName;
    }*/
    return title;
}

}


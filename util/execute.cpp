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

#include "util/execute.h"
#include <QProcess>
#include <QSystemSemaphore>
#include "TestSystemSemaphoreThread.h"
#include "../control/messagebox.h"
#include "../core/Config.h"
#include <QDir>
#include <QtGlobal>

namespace util {

static QString mwcQtWalletPath;

// By some reasons bool QProcess::startDetached(qint64 *pid = nullptr) is not in 5.9
// Guessing that something happens and it was removed. But we need it for environemnt varibales

// https://stackoverflow.com/questions/4265946/set-environment-variables-for-startdetached-qprocess
// https://bugreports.qt.io/browse/QTBUG-2284
class DetarchedProcess : public QProcess {

};



void openUrlInBrowser(const QString & url) {
#if defined(Q_OS_DARWIN)
    QProcess::startDetached("open " + url);
#elif defined(Q_OS_LINUX)
    QProcess::startDetached("xdg-open " + url);
#elif defined(Q_OS_WIN)
    Q_UNUSED(url);
    Q_ASSERT(false); // support me
#else
    Q_UNUSED(url);
    Q_ASSERT(false); // unexperted OS.
#endif
}


static QSystemSemaphore * instancesSemaphore = nullptr;

// MwcQtWallet start/stop/checking management
// Path to current instance from command line
void setMwcQtWalletPath( QString path ) {mwcQtWalletPath = path;}

static bool restartRequested = false;

// Request to restart mwc qt wallet on exit of this app
void requestRestartMwcQtWallet() {restartRequested=true;}


// Point of restart only with a gui
void restartMwcQtWalletIfRequested() {
    if (!restartRequested)
        return;

    Q_ASSERT( !mwcQtWalletPath.isEmpty() );
    QProcess::startDetached( mwcQtWalletPath, QStringList(), QDir::currentPath() );
}

// Will try to get a global lock. Return true if lock was obtained
bool acquireAppGlobalLock() {
    Q_ASSERT(instancesSemaphore==nullptr);
    instancesSemaphore = new QSystemSemaphore("mwc-qt-wallet-instances", 1);

    // Ensure that only instance can run
    // The easiest and error proof way - create global system object, so
    // OS will manage it's lifecycle.
    // Shared memory doesn't work for Linux (will work for Windows), Unfortunately it can survive crash and it is not what we need.
    // QSystemSemaphore fits better. Enen it survive crash on Unix, but OS at least revert back acquire operations.
    // It is good enough for us, so we can use QSystemSemaphore. Counter will show number of instances

    // Testing semaphore from different thread. Problem that system semaphore doesn't have timeout.
    TestSystemSemaphoreThread *testThread = new TestSystemSemaphoreThread(instancesSemaphore);
    testThread->start();
    bool testResult = true;
    if (!testThread->wait( (unsigned long)(500 * std::max(1.0,config::getTimeoutMultiplier()) + 0.5) ) ) {
            testResult = false;
    }
    delete testThread;
    return testResult;
}

// Destroy global lock object. Other instance can start once.
void releaseAppGlobalLock() {
    if (instancesSemaphore!=nullptr) {
        delete instancesSemaphore;
        instancesSemaphore = nullptr;
    }
}


}



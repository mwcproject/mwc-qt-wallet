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

#ifndef EXECUTE_H
#define EXECUTE_H

#include <QString>

// OS dependent calls
namespace util {
    void openUrlInBrowser(const QString & url);

    // MwcQtWallet start/stop/checking management
    // Path to current instance from command line
    void setMwcQtWalletPath( QString path );

    // Request to restart mwc qt wallet on exit of this app
    void requestRestartMwcQtWallet();

    // Start a new instance of the wallet. Expecting that this instance is exiting
    void restartMwcQtWalletIfRequested(double uiScale);

    // Will try to get a global lock. Return true if lock was obtained
    bool acquireAppGlobalLock();

    // Destroy global lock object. Other instance can start once.
    void releaseAppGlobalLock();

}


#endif // EXECUTE_H

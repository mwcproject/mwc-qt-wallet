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

#ifndef MWC_QT_WALLET_PROCESS_H
#define MWC_QT_WALLET_PROCESS_H

#include <QString>

class QProcess;
class QString;

namespace util {

// return true if exit successfully
bool processWaitForFinished( QProcess * process, int timeoutMs, const QString & processName );

// What current build type...
bool isBuild64Bit();

inline QString getBuildArch() { return isBuild64Bit() ? "x64" : "x32"; }


}



#endif //MWC_QT_WALLET_PROCESS_H

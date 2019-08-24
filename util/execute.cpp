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

namespace util {

void openUrlInBrowser(const QString & url) {
#if defined(Q_OS_MACOS)
    QProcess::startDetached("open " + url);
#elif defined(LINUX)
    QProcess::startDetached("xdg-open " + url);
#elif defined(Q_OS_WIN)
    Q_UNUSED(url);
    Q_ASSERT(false); // support me
#else
    Q_UNUSED(url);
    Q_ASSERT(false); // unexperted OS.
#endif


}

}



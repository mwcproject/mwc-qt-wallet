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

#include "testLogs.h"
#include <QString>
#include <QDir>
#include "../util/ioutils.h"
#include <QStringList>
#include "../core/Config.h"
#include "../util/Log.h"

namespace test {

// Write about 1 Gb of the logs and check how it will be rotated. Note, test will be slow.
// Test expect mwczip  util.
void testLogsRotation() {

    // Clean up logs first...
    QString logPath = ioutils::getAppDataPath("logs");

    QDir logDir(logPath);

    // First check if need to clean up
    QStringList logs2del = logDir.entryList();

    for (auto fn : logs2del) {
        logDir.remove(fn);
    }

    // It is a local test that is normally disabled. So using my custom build
    config::setMwcZipPath("/Users/mw/mwc713/target/release/mwczip");

    logger::initLogger(true);

    for (int t=0; t<5000000; t++) {
        logger::logInfo("testLogsRotation", "Long line " + QString::number(t) + " for testing dlksfjl kdskdsfhjflks dhfkldshf kljsdhdflkjsdhffslakjhfsdjfhdlks jfkjds fklshdfksdjhf lsdfjkdsafhsdkhfkshfkshf sjdh klsjdfhdskjhfskdjfhskjhdfskdjfhkfdsjh");
    }
}


}

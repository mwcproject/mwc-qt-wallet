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

#include "Files.h"
#include <QFile>
#include <QTextStream>

namespace util {

QStringList readTextFile( QString fileName, bool trimmed, bool cleanEMptyLines, std::function<void()> openOpenErrorCallback ) {
    QFile file(fileName);
    QStringList res;
    if (!file.open(QFile::ReadOnly)) {
        openOpenErrorCallback();
        return res;
    }

    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        if (trimmed)
            line = line.trimmed();
        if (cleanEMptyLines && line.isEmpty())
            continue;

        res << line;
    }
    file.close();
    return res;
}

// Write text lines into the file
bool writeTextFile(QString fileName, const QStringList & lines ) {

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        Q_ASSERT(false);
        return false;
    }

    QTextStream out(&file);

    for (const auto & ln : lines) {
        out << ln << endl;
    }

    file.close();
    return true;
}


}

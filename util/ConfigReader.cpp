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

#include "ConfigReader.h"
#include <QFile>
#include <QTextStream>

namespace util {

ConfigReader::ConfigReader() {}

bool ConfigReader::readConfig(QString confPath) {
    QFile inputFile(confPath);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if (line.length()<1 || line[0]=='#') // empty or comment
                continue;

            // Let's fix escaping for \\
            line.replace("\\\\", "\\");

            int idx = line.indexOf('=');
            if (idx<=0)
                continue;

            QString key = line.left(idx).trimmed();
            QString value = line.right( line.length()-idx-1 ).trimmed();
            if (key.isEmpty() )
                continue;

            if (value.size()>0 && value[0] == '"')
                value.remove(0,1);
            if (value.size()>0 && value[value.size()-1] == '"')
                value.remove(value.size()-1,1);

            config[key] = value;
        }
        inputFile.close();
    }
    else
        return false;

    return true;
}

bool ConfigReader::isDefined(QString key) const {
    return config.contains(key);
}

QString ConfigReader::getString(QString key) const {
    return config.value(key);
}


}


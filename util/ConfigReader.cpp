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
#include "Files.h"

namespace util {

QString unwrapQuotes(QString value) {
    if (value.size()>0 && value[0] == '"')
        value.remove(0,1);
    if (value.size()>0 && value[value.size()-1] == '"')
        value.remove(value.size()-1,1);

    return value;
}

ConfigReader::ConfigReader() {}

bool ConfigReader::readConfig( const QString & confPath ) {
    configFN = confPath;
    QFile inputFile(confPath);
    lines.clear();

    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();

            lines.push_back(line);

            if (line.length()<1 || line[0]=='#') // empty or comment
                continue;

            // Let's fix escaping for '\\'
            line.replace("\\\\", "\\");

            int idx = line.indexOf('=');
            if (idx<=0)
                continue;

            QString key = line.left(idx).trimmed();
            QString value = line.right( line.length()-idx-1 ).trimmed();
            if (key.isEmpty() )
                continue;

            value = unwrapQuotes(value);
            config[key] = KeyInfo( key, value, lines.size()-1 );
        }
        inputFile.close();
    }
    else
        return false;

    return true;
}

bool ConfigReader::isDefined( const QString & key ) const {
    return config.contains(key);
}

QString ConfigReader::getString( const QString & key ) const {
    return config.value(key).value;
}

// Update the value and rewrite the config.
// Many IOs are not expected
// Note!!!!  value MUST respoct toml even it is not critical for now
bool ConfigReader::updateConfig( const QString & key, const QString & value ) {
    QString configLine = key + " = " + value;
    if ( config.contains(key) ) {
        KeyInfo & ki = config[key];
        ki.value = unwrapQuotes(value);
        lines[ki.lineIdx] = configLine;
    }
    else {
        lines.push_back( configLine );
        config[key] = KeyInfo( key, value, lines.size()-1 );
    }

    return writeTextFile( configFN, lines );
}


}


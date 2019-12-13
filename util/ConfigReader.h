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

#ifndef GUI_WALLET_CONFIGREADER_H
#define GUI_WALLET_CONFIGREADER_H

#include <QMap>

namespace util {

struct KeyInfo {
    QString key;
    QString value;
    int lineIdx = 0;

    KeyInfo(QString _key, QString _value, int _lineIdx) : key(_key), value(_value), lineIdx(_lineIdx) {}

    KeyInfo() = default;
    KeyInfo( const KeyInfo & other ) = default;
    KeyInfo & operator = ( const KeyInfo & other ) = default;
};

class ConfigReader {
public:
    ConfigReader();

    // Config is reading toml format. We are persing it manually, so details might be different from toml
    bool readConfig(const QString & confPath);

    bool isDefined(const QString & key) const;
    QString getString(const QString & key) const;

    // Update the value and rewrite the config.
    // Many IOs are not expected
    // Note!!!!  value MUST respoct toml even it is not critical for now
    bool updateConfig( const QString & key, const QString & value );
private:
    QString configFN;
    QMap<QString, KeyInfo>  config;
    QStringList lines;
};

}

#endif //GUI_WALLET_CONFIGREADER_H

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

#include "MessageMapper.h"
#include "../util/Files.h"

namespace test {
void testMessageMapper() {
    notify::MessageMapper mapper(":/resource/mwc713_mappers.txt");

    Q_ASSERT( mapper.processMessage("Some normal message") == "Some normal message");
    Q_ASSERT(mapper.processMessage("Swap Error , Electrum Node error, Unable to contact the secondary ElectrumX client btc.test2.swap.mwc.mw:8000, Swap Rpc error: Unable connect to btc.test2.swap.mwc.mw:8000, Swap I/O: Connection refused (os error 61)") ==
                "The secondary ElectrumX client is not accessible. Unable connect to btc.test2.swap.mwc.mw:8000, Connection refused (os error 61)");
}
}

namespace notify {

void Mapper::init(const QString & regexPattern, const QString & _mapper ) {
    parser.setPattern(regexPattern);
    mapper = _mapper;
}

// Parse and return string if pattern match. Otherwise return the empty value
QString Mapper::process(const QString & str) const {
     QRegularExpressionMatch match = parser.match(str);
     if (!match.hasMatch())
         return "";

     QString resStr = mapper;
     // Substitute groups until we don't have 2 in the row

     int notFound = 0;
     for (int grIdx=1; grIdx<100 && notFound<2; grIdx++) {
         QString groupPattern = "$" + QString::number(grIdx);
         int idx = resStr.indexOf(groupPattern);
         if (idx<0) {
             notFound++;
             continue;
         }

         notFound=0;
         resStr.replace(groupPattern, match.captured(grIdx));
     }

     return resStr;
}

MessageMapper::MessageMapper(const QString & fileName) {
    readMappingConfig(fileName);
}

MessageMapper::~MessageMapper() {
}

QString MessageMapper::processMessage(const QString & message) const {
    for (const auto & m : mappers) {
        QString res = m.process(message);
        if (!res.isEmpty())
            return res;
    }
    return message;
}

// Reading config with regular expressions.
void MessageMapper::readMappingConfig(const QString & configFileName) {
    QStringList lns = util::readTextFile( configFileName, true, false, [](){Q_ASSERT(false);});

    Q_ASSERT(!lns.isEmpty());

    for (int q=0; q<lns.size()-1; q++ ) {
        const QString & l = lns[q];
        if (l.isEmpty() || l.startsWith('#'))
            continue; // it is comment.

        // That must be a parser Data. Next two lines.

        q++;
        const QString & mapperLine = lns[q];
        Q_ASSERT( !mapperLine.isEmpty() && !mapperLine.startsWith('#') );

        Mapper mpr;
        mpr.init(l, mapperLine);
        mappers.push_back(mpr);
    }
}

}

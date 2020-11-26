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

#ifndef MWC_QT_WALLET_MESSAGEMAPPER_H
#define MWC_QT_WALLET_MESSAGEMAPPER_H

#include <QString>
#include <QRegularExpression>
#include <QVector>

namespace notify {

// Message mapper needed for notificaitons mapping. We want to handle messages like this:
// Swap Error , Electrum Node error, Unable to contact the secondary ElectrumX client btc.test2.swap.mwc.mw:8000, Swap Rpc error: Unable connect to btc.test2.swap.mwc.mw:8000, Swap I/O: Connection refused (os error 61)
//
// Mapper is providing bunch of regex parsers that will be apply to every income messge.
// Note!!!  Parsers are applied one by one, so performance is limited. For notifications it is good enough.
//          But if you want to handle high traffic, please concider to use one parser for all.
// Note!!!  This mapper has tests, maintain it for every new mapper!!!!
struct Mapper {
    QRegularExpression parser;
    QString mapper;

    void init(const QString & regexPattern, const QString & mapper );

    // Parse and return string if pattern match. Otherwise return the empty value
    QString process(const QString & str) const;
};

class MessageMapper {
public:
    MessageMapper(const QString & configFileName);
    ~MessageMapper();

    // Proces sthe messag eand return the result.
    QString processMessage(QString message) const;
private:
    // Reading config with regular expressions.
    void readMappingConfig(const QString & fileName);

private:
    QVector<Mapper> mappers;
};

}

// Maintain me!!!
namespace test {
    void testMessageMapper();
}


#endif //MWC_QT_WALLET_MESSAGEMAPPER_H

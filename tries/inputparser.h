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

#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <QVector>
#include "baseparser.h"
#include <QDebug>

namespace tries {

class TrieLineParser;

struct ParsingResult {
    const int parserId = -1;
    LineResult result;

    ParsingResult() = default;
    ParsingResult(const ParsingResult & other) = default;

    ParsingResult(int parserId, const LineResult & result);

    ParsingResult & operator=(const ParsingResult & other) = default;

};

QDebug operator<<(QDebug dbg, const ParsingResult& res);

struct LineInfo {
    TrieLineParser * const parser;
    TrieLineContext * const context;

    LineInfo(TrieLineParser * _parser, TrieLineContext * _context) : parser(_parser), context(_context) {}
    LineInfo() : parser(nullptr), context(nullptr) {}
    LineInfo(LineInfo && item) = default;
    LineInfo(const LineInfo & item) = default;
};

class InputParser
{
public:
    InputParser();
    ~InputParser();

    InputParser(const InputParser & other) = delete;
    InputParser & operator=(const InputParser & other) = delete;

    // append a new line parser.
    // parser must be on the heap and pnership will be transferred to this
    void appendLineParser( TrieLineParser* parser );

    // return true if anything was deleted
    bool deleteLineParser(int parserId);

    QVector<ParsingResult> processInput(QString input);

protected:
    QVector< LineInfo > lines;
};

}

#endif // INPUTPARSER_H

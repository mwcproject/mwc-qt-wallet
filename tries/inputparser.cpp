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

#include "tries/inputparser.h"

namespace tries {

ParsingResult::ParsingResult(int _parserId, const LineResult & _result) :
    parserId(_parserId), result(_result)
{
}


QDebug operator<<(QDebug dbg, const ParsingResult& res)
{
    dbg << "ParsingResult(parserId=" << res.parserId << ",result=" << res.result << ")";
    return dbg;
}


InputParser::InputParser() {
}

InputParser::~InputParser() {
    for ( const auto & p : lines) {
        delete p.parser;
        delete p.context;
    }
    lines.clear();
}

// append a new line parser.
// parser must be on the heap and pnership will be transferred to this
void InputParser::appendLineParser( TrieLineParser* parser, bool hasSingleActiveContext ) {
    lines.push_back( LineInfo( parser, new TrieLineContext(hasSingleActiveContext) ) );
}

bool InputParser::deleteLineParser(int parserId) {
    int sz = lines.size();
    for (int t=sz-1; t>=0; t--) {
        if (lines[t].parser->getParserId() == parserId) {
            delete lines[t].parser;
            delete lines[t].context;
            lines.remove(t);
        }
    }
    return lines.size() < sz;
}

QVector<ParsingResult> InputParser::processInput(QString input) {
    // processing input symbol by symbol
    int len = input.length();

    QVector<ParsingResult> result;

    for ( int l=0; l<len; l++ ) {
        QChar ch = input[l];

        for ( LineInfo & p : lines ) {
            if (p.parser->process(ch, p.context))
            { // get a result...
                const QVector<LineResult> & res = p.context->getReadyResult();
                for ( auto & r : res ) {
                    result.push_back( ParsingResult(p.parser->getParserId(), r ) );
                }
                p.context->resetResults();

                // not resetting statuses for the rest.
                // Naturally reset should be atchieved.
            }
        }
    }
    return result;
}


}

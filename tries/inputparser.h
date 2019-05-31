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

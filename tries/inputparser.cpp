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
void InputParser::appendLineParser( TrieLineParser* parser ) {
    lines.push_back( LineInfo( parser, new TrieLineContext() ) );
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

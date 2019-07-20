#include "tries/baseparser.h"

namespace tries {

LineResult::LineResult(const LineResult & other, SectionResult && next ) :
    parseResult(other.parseResult)
{
    parseResult.append(next);
}


QDebug operator<<(QDebug dbg, const LineResult& res) {
    dbg << "LineResult(";
    for (auto & r: res.parseResult) {
       dbg << r.strData << "," << r.dataId << "; ";
    }
    dbg << ")";
    return dbg;
}


BaseTrieSection::BaseTrieSection(int _accumulateId) : accumulateId(_accumulateId) {}
BaseTrieSection::~BaseTrieSection() {}

void BaseTrieSection::setParentParsers(BaseTrieSection * _prevParser) {
    prevParser = _prevParser;
}

void BaseTrieSection::appendNextParsers(BaseTrieSection * _nextParser) {
    nextParser.push_back(_nextParser);
}

// Current section context
TrieSectionContext::TrieSectionContext(BaseTrieSection * const _section, const LineResult & prevResult) :
    prevParseResults(prevResult),
    section(_section)
{
    accId = section->getAccumulateId();
}

// return PROCESS_RESULT flags
uint32_t TrieSectionContext::processChar( QChar ch )
{
    uint32_t res = section->processChar(sectionContext, ch);

    // Note, last symbol need to be excluded
    if (accId>=0)
        accStr += ch;

    return res;
}

LineResult TrieSectionContext::calcResult() const {
    if (accId<0)
        return prevParseResults;

    return LineResult(prevParseResults, SectionResult(accStr.left( std::max(accStr.size()-1,0) ) ,accId) );
}


// Context from the whole line
TrieLineContext::TrieLineContext() {}

TrieLineContext::~TrieLineContext() {
    releaseData();
}

void TrieLineContext::releaseData() {
    for (auto cnt : contexts) {
        delete cnt;
    }
    contexts.clear();
}

void TrieLineContext::reset() {
    releaseData();
}

// Process char against whole context
void TrieLineContext::processChar( BaseTrieSection * headSection, QChar ch ) {
    // Process what we already have

    int sz = contexts.size();
    for ( int i=sz-1; i>=0; i-- ) {
        TrieSectionContext* cont = contexts[i];
        uint32_t res = cont->processChar(ch);

        if ( res & (BaseTrieSection::PROCESS_RESULT::DONE | BaseTrieSection::PROCESS_RESULT::START_NEXT) ) {
            processContext(cont, (res & BaseTrieSection::PROCESS_RESULT::DONE)!=0,
                           (res & BaseTrieSection::PROCESS_RESULT::START_NEXT)!=0, ch);

        }

        if ( res & BaseTrieSection::PROCESS_RESULT::KEEP ) {
            continue; // Keeping this object
        }

        // FAIL, DONE(not Keep) should be here
        delete cont;
        contexts.remove(i);
    }

    // Every iteration try to start a new parser
    startNewSectionAndProcess( headSection, LineResult(), ch );
}

void TrieLineContext::processContext(TrieSectionContext* context, bool done, bool startNext, QChar ch) {
    Q_ASSERT(done || startNext);
    const QVector<BaseTrieSection*> & next = context->getNextSections();

    if (next.empty()) { // last in the chain - mean we can get results, but we can't spawn new sections
        if ( done )
            readyResult.push_back( context->calcResult() );
        return;
    }

    if (startNext) {
        for (auto ns : next) {
            startNewSectionAndProcess( ns, context->calcResult(), ch );
        }
    }
    else {
        // Just append new sections
        for (auto ns : next) {
            contexts.push_back( new TrieSectionContext(ns, context->calcResult()) );
        }
    }
}

void TrieLineContext::startNewSectionAndProcess( BaseTrieSection * newSection, LineResult && prevResult, QChar ch ) {

    { // preliminary test. Just test a symbol and discard if no match
        TrieContext tc;
        uint32_t res = newSection->processChar(tc, ch);

        // Can't have double Next, should expect at least 1 symbol
        Q_ASSERT( (res & BaseTrieSection::PROCESS_RESULT::START_NEXT) == 0  );

        if (res==BaseTrieSection::PROCESS_RESULT::FAIL)
            return;
    }

    TrieSectionContext * newContext = new TrieSectionContext(newSection, prevResult );
    uint32_t res = newContext->processChar(ch);

    if ( res & (BaseTrieSection::PROCESS_RESULT::DONE | BaseTrieSection::PROCESS_RESULT::START_NEXT) ) {
        processContext(newContext, (res & BaseTrieSection::PROCESS_RESULT::DONE)!=0,
                       (res & BaseTrieSection::PROCESS_RESULT::START_NEXT)!=0, ch);

    }

    // keeping a new context
    if ( res & BaseTrieSection::PROCESS_RESULT::KEEP ) {
        contexts.push_back( newContext );
        return;
    }

    // Seems like nobody need that. Deleting the context
    delete newContext;
}


// Engine that parsing single intput until it gets output
//
TrieLineParser::TrieLineParser(int _parserId) :
    parserId(_parserId)
{}

TrieLineParser::TrieLineParser(int _parserId, QVector<BaseTrieSection*> && ss)  :
    parserId(_parserId), sections(ss)
{
    connectLineSections( sections.size() - ss.size() );
}

TrieLineParser::~TrieLineParser() {
    for (auto s : sections) {
        delete s;
    }
    sections.clear();
}

// Adding section/line for parsing and pair them in chain
TrieLineParser & TrieLineParser::addSection( BaseTrieSection* s ) {
    sections.push_back(s);
    connectLineSections( sections.size() - 1 );
    return *this;
}

TrieLineParser & TrieLineParser::addLine( QVector<BaseTrieSection*> ss ) {
    sections.append(ss);
    connectLineSections( sections.size() - ss.size() );
    return *this;
}

// Put sections in line
void TrieLineParser::connectLineSections( int lastAddIdx ) {
    int idxFrom = std::max(0, lastAddIdx-1);

    for ( int t=idxFrom+1; t<sections.size(); t++ ) {
        BaseTrieSection * s1 = sections[t-1];
        BaseTrieSection * s2 = sections[t];

        s2->setParentParsers(s1);
        s1->appendNextParsers(s2);
    }
}

// return true if TrieLineContext has some result;
bool TrieLineParser::process( QChar ch, TrieLineContext * context ) {
    Q_ASSERT( sections.size()>0 );
    context->processChar( sections.front(), ch );
    return context->hasResults();
}


}

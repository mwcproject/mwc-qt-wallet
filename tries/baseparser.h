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

#ifndef BASEPARSER_H
#define BASEPARSER_H

#include <QString>
#include <QVector>
#include <QDebug>

namespace tries {

// Ready result from a single BaseTrieSection
class SectionResult {
public:
    SectionResult() = default;
    SectionResult(QString _strData, int _dataId) : strData(_strData), dataId(_dataId) {}

    QString strData;
    int dataId = 0;
};

// Ready results from single chain (line)
class LineResult {
public:
    LineResult() = default;
    ~LineResult() = default;
    LineResult(const LineResult &) = default;

    LineResult(const LineResult & other, SectionResult && next );

    LineResult & operator=(const LineResult &) = default;

    void AddResult(const SectionResult & res) {parseResult.push_back(res);}

    QVector<SectionResult> parseResult;
};

QDebug operator<<(QDebug dbg, const LineResult& res);

// Context for a single trie
class TrieContext {
public:
    int      pos  = 0;
    uint32_t mask = 0;
};

// Base class for trie item. Expected that it can be in focus and it can delegate focus
// to the next one. Example: number, any at least 30 symbols. e.t.c
class BaseTrieSection
{
public:
    BaseTrieSection(int accumulateId);
    virtual ~BaseTrieSection();

    BaseTrieSection(const BaseTrieSection&) = delete;
    BaseTrieSection & operator=(const BaseTrieSection&) = delete;

    // return -1 if no need to accumultate the results.
    // Otherwise return accumulate Id
    int getAccumulateId() const {return accumulateId;}

    // Process char with a data.
    // DONE - finished.
    // START_NEXT - this char wan't processed, need to process with next
    enum PROCESS_RESULT { FAIL = 0, DONE = 0x01, KEEP = 0x02, START_NEXT = 0x04 };
    virtual uint32_t processChar(TrieContext & context, QChar ch) = 0;

    void setParentParsers(BaseTrieSection * prevParser);
    void appendNextParsers(BaseTrieSection * nextParser);

    BaseTrieSection * getPrevParser() const { return prevParser; }
    const QVector<BaseTrieSection*> & getNextParser() const { return nextParser; }

protected:
    int accumulateId;
    BaseTrieSection * prevParser = nullptr; // Parent parser in the chain, can be only one
    QVector<BaseTrieSection*> nextParser; // Next parsers in the chain. Can be many
};

// Current section context
class TrieSectionContext {
public:
    TrieSectionContext(BaseTrieSection * const section, const LineResult & prevResult );

    // return PROCESS_RESULT flags
    uint32_t processChar( QChar ch );

    // Calculate results for current state
    LineResult calcResult() const;

    const QVector<BaseTrieSection*> & getNextSections() const { return section->getNextParser(); }

protected:
    LineResult        prevParseResults;
    BaseTrieSection * const section; // section related to the context.
    int               accId;      // Accumulator ID. If negative - no accumulation need to be made.
    QString           accStr;     // Accumulated String.
    TrieContext       sectionContext;
};

// Context from the whole line
class TrieLineContext {
public:
    TrieLineContext(bool hasSingleActiveContext);
    ~TrieLineContext();

    TrieLineContext(const TrieLineContext&) = delete;
    TrieLineContext & operator=(const TrieLineContext&) = delete;

    // Process char against whole context
    void processChar( BaseTrieSection * headSection, QChar ch );

    void reset(); // Reset whole context

    bool hasResults() const {return !readyResult.empty();}
    const QVector<LineResult> & getReadyResult() const {return readyResult;}
    // Clean up the result that we get from ready
    void resetResults() {readyResult.clear();}
private:
    void releaseData();
    void processContext(TrieSectionContext* context, bool done, bool startNext, QChar ch);
    void startNewSectionAndProcess( BaseTrieSection * newSection, LineResult && prevResult, QChar ch );
protected:
    bool isSingleActiveContext;
    QVector< TrieSectionContext* > contexts; // owners those objects
    QVector<LineResult> readyResult;
};


// Engine that parsing single intput until it gets output
//
class TrieLineParser {
public:
    TrieLineParser(int parserId);
    TrieLineParser(int parserId, QVector<BaseTrieSection*> && ss);
    ~TrieLineParser();

    TrieLineParser(const TrieLineParser & other) = delete;
    TrieLineParser & operator=(const TrieLineParser & other) = delete;

    int getParserId() const {return parserId;}

    // Adding section/line for parsing and pair them in chain
    TrieLineParser & addSection( BaseTrieSection* s );
    TrieLineParser & addLine( QVector<BaseTrieSection*> ss );
    // Non chain topology still need to be added

    // return true if TrieLineContext has some result;
    bool process( QChar ch, TrieLineContext * context );
private:
    // Put sections in line
    void connectLineSections( int lastAddIdx );
protected:
    const int parserId;
    QVector< BaseTrieSection* > sections; // Owns section instances
};

}


#endif // BASEPARSER_H

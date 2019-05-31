#ifndef SIMPLETRIESECTION_H
#define SIMPLETRIESECTION_H

#include "baseparser.h"

namespace tries {

// Parsing the fixed phrase
class TriePhaseSection : public BaseTrieSection {
public:
    TriePhaseSection(QString phrase, int accumulateId=-1);

    virtual uint32_t processChar(TrieContext & context, QChar ch) override;
protected:
    QString phrase;
};

// Parsing the version like: '2.0.0'
class TrieVersionSection : public BaseTrieSection {
public:
    TrieVersionSection(int accumulateId = -1);

    virtual uint32_t processChar(TrieContext & context, QChar ch) override;
protected:
};

// Just a new line
class TrieNewLineSection : public BaseTrieSection {
public:
    TrieNewLineSection();
    virtual uint32_t processChar(TrieContext & context, QChar ch) override;
};

// Anything that match the set of letters
class TrieAnySection : public BaseTrieSection {
public:
    // lenLimit - max length of the phrase
    enum PROCESS { NUMBERS=0x0001, LOW_CASE=0x0002, UPPER_CASE=0x0004, SPACES=0x0008,
                   ANY_UNTIL_NEW_LINE = 0x0010,
                   NEW_LINE=0x0100, BRACKETS=0x0200 };
    TrieAnySection(int lenLimit, uint32_t processMask, int accumulateId=-1 );
    virtual uint32_t processChar(TrieContext & context, QChar ch) override;
protected:
    int lenLimit;
    uint32_t processMask;
};


}

#endif // SIMPLETRIESECTION_H

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

#ifndef SIMPLETRIESECTION_H
#define SIMPLETRIESECTION_H

#include "baseparser.h"

namespace tries {

// Parsing the fixed phrase
class TriePhraseSection : public BaseTrieSection {
public:
    TriePhraseSection(QString phrase, int accumulateId=-1);
    TriePhraseSection(QString phrase, bool ignoreCase, int accumulateId=-1);

    virtual uint32_t processChar(TrieContext & context, QChar ch) override;
protected:
    QString phrase;
    bool ignoreCase = false;
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
                   NOT_NEW_LINE = 0x0010, NOT_SPACES=0x0020,
                   NEW_LINE=0x0100,
                   START_NEXT_EVERY_TRY=0x1000 };
    // acceptSymbols & stopSymbols are char sets. If strings are empty, will be ignored
    TrieAnySection(int lenLimit, uint32_t processMask, QString acceptSymbols, QString stopSymbols, int accumulateId=-1 );
    virtual uint32_t processChar(TrieContext & context, QChar ch) override;
protected:
    int lenLimit;
    uint32_t processMask;
    QString acceptSymbols;
    QString stopSymbols;
};


}

#endif // SIMPLETRIESECTION_H

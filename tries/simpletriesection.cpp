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

#include "tries/simpletriesection.h"

namespace tries {


// Parsing the fixed phrase
TriePhraseSection::TriePhraseSection(QString _phrase, int accumulateId) :
    BaseTrieSection(accumulateId), phrase(_phrase)
{}

uint32_t TriePhraseSection::processChar(TrieContext & context, QChar ch) {
    Q_ASSERT( context.pos>=0 );
    Q_ASSERT( context.pos < phrase.length() );

    bool ok = (ch == phrase[context.pos]);
    if (!ok)
        return PROCESS_RESULT::FAIL;

    uint32_t res = 0;

    context.pos++;

    if (context.pos<phrase.length())
        res |= PROCESS_RESULT::KEEP;
    else
        res |= PROCESS_RESULT::DONE;

    return res;
}

// Parsing the version like: '2.0.0'
TrieVersionSection::TrieVersionSection(int accumulateId) :
    BaseTrieSection(accumulateId)
{}

uint32_t TrieVersionSection::processChar(TrieContext & context, QChar ch) {
    // we don't case about the context. Just accept digits and '.'

    bool ok = (ch.isDigit() || (ch == QChar('.')) || (ch == QChar('-')) || (ch.isLetter()));

    if (ok) {
        context.pos++;
        return PROCESS_RESULT::KEEP;
    }
    else {
        if (context.pos==0)
            return PROCESS_RESULT::FAIL;
        // Current char still need to be processed.
        return PROCESS_RESULT::DONE | PROCESS_RESULT::START_NEXT;
    }
}

// Just a new line
TrieNewLineSection::TrieNewLineSection() : BaseTrieSection(-1) {}

uint32_t TrieNewLineSection::processChar(TrieContext & context, QChar ch) {
    Q_UNUSED(context);

    if ( ch==QChar::LineSeparator || ch==QChar::LineFeed ||ch==QChar::CarriageReturn )
        return PROCESS_RESULT::DONE;
    else
        return PROCESS_RESULT::FAIL;
}

TrieAnySection::TrieAnySection(int _lenLimit, uint32_t _processMask,
        QString _acceptSymbols, QString _stopSymbols,
        int accumulateId ) :
    BaseTrieSection( accumulateId ),
    lenLimit(_lenLimit),
    processMask(_processMask),
    acceptSymbols(_acceptSymbols),
    stopSymbols(_stopSymbols)
{
}

inline bool isNewLine(QChar ch) {
    return ch==QChar::LineSeparator || ch==QChar::LineFeed || ch==QChar::CarriageReturn;
}

uint32_t TrieAnySection::processChar(TrieContext & context, QChar ch) {
    bool ok = false;
    if (processMask & PROCESS::NUMBERS)
        ok = ok || ch.isDigit();

    if (processMask & PROCESS::LOW_CASE)
        ok = ok || (ch.isLetter() && ch.isLower());

    if (processMask & PROCESS::UPPER_CASE)
        ok = ok || (ch.isLetter() && ch.isUpper());

    if (processMask & PROCESS::SPACES)
        ok = ok || ch.isSpace();

    if (processMask & PROCESS::NOT_SPACES)
        ok = ok || !ch.isSpace();

    if (processMask & PROCESS::NOT_NEW_LINE)
        ok = ok || !isNewLine(ch);

    if (processMask & PROCESS::NEW_LINE)
        ok = ok || isNewLine(ch);

    ok = ok || acceptSymbols.contains(ch);

    // stop must be the last
    if (stopSymbols.contains(ch))
        ok = false;

    uint32_t startNextMask = 0;
    if (processMask & START_NEXT_EVERY_TRY){
        startNextMask = PROCESS_RESULT::START_NEXT;
    }

    if (ok) {
        context.pos++;
        return PROCESS_RESULT::KEEP | startNextMask;
    }

    if (context.pos==0)
        return PROCESS_RESULT::FAIL;

    // Done and process with next
    context.pos++;
    return (PROCESS_RESULT::DONE | PROCESS_RESULT::START_NEXT);
}

}

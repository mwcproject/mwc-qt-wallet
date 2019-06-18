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

TrieAnySection::TrieAnySection(int _lenLimit, uint32_t _processMask, int accumulateId ) :
    BaseTrieSection( accumulateId ),
    lenLimit(_lenLimit),
    processMask(_processMask)
{
}

const static QSet<QChar> BRACKETS_CHAR{ QChar('['), QChar(']'), QChar('{'), QChar('}'), QChar('('), QChar(')') };

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

    if (processMask & PROCESS::ANY_UNTIL_NEW_LINE)
        ok = ok || !isNewLine(ch);

    if (processMask & PROCESS::NEW_LINE)
        ok = ok || isNewLine(ch);

    if (processMask & PROCESS::BRACKETS)
        ok = ok || BRACKETS_CHAR.contains(ch);

    if (ok) {
        context.pos++;
        return PROCESS_RESULT::KEEP;
    }

    if (context.pos==0)
        return PROCESS_RESULT::FAIL;

    // Done and process with next
    context.pos++;
    return (PROCESS_RESULT::DONE | PROCESS_RESULT::START_NEXT);
}

}

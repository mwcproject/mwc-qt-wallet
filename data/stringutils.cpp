#include "stringutils.h"


QVector<QString> parsePhrase2Words( QString phrase ) {

    // input seed in low cases
    QVector<QString> words;
    words.push_back("");

    for (QChar ch : phrase) {
        if  (ch.isLetter()) { // letter goes to the last word
            words.back()+=ch;
        }
        else {
            // Start new word
            if (words.back().length()==0)
                continue;
            words.push_back("");
        }
    }

    // Clean up the last seprator case
    if (words.back().length()==0)
        words.pop_back();

    return words;
}

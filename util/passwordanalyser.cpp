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

#include "passwordanalyser.h"
#include <QMap>
#include <math.h>

namespace util {


PasswordAnalyser::PasswordAnalyser(QString _attentinColor, QString _happyColor ) :
        attentinColor(_attentinColor),
        happyColor(_happyColor),
        sequenceAnalyzer( dict::buldPasswordChackWordSequences() ) {

    Q_ASSERT(DICTS_NUM==4);

    dictionaries[0] = new dict::WordDictionary(":/resource/passwords-1k.dat");
    dictionaryWeight[0] = 1.0; // 1 k include 10 & 100.  Let's ban it to one symbol. In any case it is lett than 2 symbols

    dictionaries[1] = new dict::WordDictionary(":/resource/passwords-10k.dat");
    dictionaryWeight[1] = 2.0; // 13.2 bits

    dictionaries[2] = new dict::WordDictionary(":/resource/passwords-100k.dat");
    dictionaryWeight[2] = 2.5; // 16.6 bits  (7 per char is ok)

    dictionaries[3] = new dict::WordDictionary(":/resource/passwords-1M.dat");
    dictionaryWeight[3] = 3.0; // 20 bits
}

PasswordAnalyser::~PasswordAnalyser() {
    for ( auto d : dictionaries ) {
        delete d;
    }
}

// return String to print
QPair<QString, bool> PasswordAnalyser::getPasswordQualityReport(const QString & pass, // in
              QVector<double> & weight,
              QStringList & seqWords,
              QStringList & dictWords)
{
    // mwc713 password can't from '-',  config parser doesn't handle that
    if (pass.startsWith("-")) {
        return QPair<QString, bool>("<font color="+attentinColor+">Password can't be started from '-' symbol.</font>", false);
    }

    // Check if all capital or lower case
    bool hasUpperCase = false;
    bool hasLowerCase = false;
    bool hasNumbers = false;
    bool hasSpecialSymbols = false;
    for (auto ch : pass) {
        if (ch.isLetter()) {
            if (ch.isUpper())
                hasUpperCase = true;
            else
                hasLowerCase = true;
            continue;
        }

        if (ch.isNumber()) {
            hasNumbers = true;
            continue;
        }

        hasSpecialSymbols = true;
    }

    int alphabetSz = 1;
    if (hasUpperCase)
        alphabetSz += 32;
    if (hasLowerCase)
        alphabetSz += 32;
    if (hasNumbers)
        alphabetSz += 10;
    if (hasSpecialSymbols)
        alphabetSz += 32;

    // Minimum number of effective bits needed for the password to pass.
    double bitsMinSum = log2( 32.0 * 3 + 10 ) * 7;

    double singleCharBitWeight = log2(alphabetSz);

    // init all letters with a start weight
    weight.resize(pass.length());
    for (auto & w : weight)
        w = singleCharBitWeight;

    seqWords.clear();
    dictWords.clear();

    if (pass.size()<PASS_MIN_LEN)
        return QPair<QString, bool>("<font color="+attentinColor+">Password must be at least "+
                 QString::number(PASS_MIN_LEN)+" symbols</font>", false);

    // Let's check for sequences. All sequence has a weight 1
    for ( auto s : sequenceAnalyzer.detectSequences(pass, weight, singleCharBitWeight) )
        if (s.length()>2)
            seqWords << s;

    // Let's check dictionary words
    for ( int t=0; t<DICTS_NUM; t++ ) {
        dictWords += dictionaries[t]->detectDictionaryWords(pass, weight, dictionaryWeight[t] * 7.0 ); // dictionary has the full alphabet - 7 bits
    }

    // Let's pack the dictionary words...
    for (int i=dictWords.size()-1; i>=0; i--) {

        bool included = false;
        for (int j = 0; j < dictWords.size(); j++) {
            if (i == j)
                continue;

            included = included || dictWords[j].contains(dictWords[i]);
        }
        if (included)
            dictWords.removeAt(i);
    }

    // Check if the password good enough
    double weightsSum = 0.0;
    for (auto w : weight)
        weightsSum += w;

    if (weightsSum+0.01 >= bitsMinSum) {
        // we are good to go!
        if ( seqWords.isEmpty() && dictWords.isEmpty() )
            return QPair<QString, bool>("",true); // Great password

        QString respondStr = "<font color="+happyColor+">Your password is acceptable. But please note that it has:";
        if (seqWords.size()>0)
            respondStr += "<br>Sequences: " + seqWords.join(", ");
        if (dictWords.size()>0)
            respondStr += "<br>Dictionary words: " + dictWords.join(", ");

        respondStr += "</font>";

        return QPair<QString, bool>(respondStr, true);
    }

    // Here we are not good to go at all.
    if ( seqWords.isEmpty() && dictWords.isEmpty() ) {
        return QPair<QString, bool>("<font color="+attentinColor+">Please specify longer password or use more symbols variation.</font>", false);
    }

    QString respondStr = "<font color="+attentinColor+">Please specify longer password or don't use:";
    if (seqWords.size()>0)
        respondStr += "<br>Sequences: " + seqWords.join(", ");
    if (dictWords.size()>0)
        respondStr += "<br>Dictionary words: " + dictWords.join(", ");

    respondStr += "</font>";

    return QPair<QString, bool>(respondStr, false);
}

}

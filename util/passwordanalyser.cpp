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

namespace util {

const int PASS_MIN_LEN      = 10;
const int PASS_MIN_NON_LETTER = 1;
const int PASS_MIN_CAPITAL  = 1;
const int PASS_MIN_LOWCASE  = 1;

PasswordAnalyser::PasswordAnalyser(const QString & password) :
    pass2check(password)
{
    len = pass2check.length();

    letters = nonLetters = capitals = locase = 0;

    // want to keep keys sorted
    QMap<int, int> entropy;
    for ( QChar ch : pass2check )
    {
        entropy[ch.unicode()]=ch.unicode();
        if (ch.isLetter()) {
            letters++;

            if (ch.isUpper())
                capitals++;
            else
                locase++;
        }
        else {
            nonLetters++;
        }
    }
    if (entropy.size()<1)
        entropy[0]=0;

    // Calculating entropy by apply the same operation once (twice is too strict)
    for (int t=0;t<1;t++)
    {
        QMap<int, int> buffer;
        buffer.swap(entropy);

        Q_ASSERT(entropy.size()==0);

        int prev=0;
        for ( auto bi = buffer.begin(); bi != buffer.end(); bi++ ) {
            int diff = bi.key() - prev;
            Q_ASSERT(diff>=0);
            entropy[ diff ] = diff;
            prev = bi.key();
        }
    }

    entropyLen = entropy.size();
}


// return string that describe the quality of the password
// Example: <font color="red">aaaa</font>
QString PasswordAnalyser::getPasswordQualityStr()
{
    if (len<PASS_MIN_LEN)
        return "<font color=#CCFF33>Password minimum length is "+QString::number(PASS_MIN_LEN)+" symbols</font>";

    if (nonLetters<PASS_MIN_NON_LETTER )
        return "<font color=#CCFF33>Password require at least "+ QString::number(PASS_MIN_NON_LETTER) +" non letter symbols</font>";

    if (capitals<PASS_MIN_CAPITAL )
        return "<font color=#CCFF33>Password require at least "+ QString::number(PASS_MIN_CAPITAL) +" capital letter</font>";

    if (locase<PASS_MIN_LOWCASE )
        return "<font color=#CCFF33>Password require at least "+ QString::number(PASS_MIN_LOWCASE) +" low case symbol</font>";

    if (entropyLen < len/2) {
        return "<font color=#CCFF33>Your password phrase has low entropy. Please choose another phrase</font>";
    }

    return "";// "<font color=\"green\">OK</font>";
}

bool PasswordAnalyser::isPasswordOK() const {
    return len>=PASS_MIN_LEN && nonLetters>=PASS_MIN_NON_LETTER  &&
            capitals >= PASS_MIN_CAPITAL && locase>=PASS_MIN_LOWCASE && entropyLen >= len/2;
}

}

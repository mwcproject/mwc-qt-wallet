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

#include "testPasswordAnalyser.h"
#include "../util/passwordanalyser.h"

namespace test {

void testPasswordAnalyser() {
    util::PasswordAnalyser pa;

    QVector<double> weight;
    QStringList seqWords, dictWords;


    // return html string with a success flag that describe the quality of the password
    // Example: <font color=#CCFF33>aaaa</font>
    QPair<QString, bool> res = pa.getPasswordQualityReport( "victor", weight, seqWords, dictWords);
    Q_ASSERT(!res.second); // too short

    res = pa.getPasswordQualityReport( "123456fghjkpoiuyt", weight, seqWords, dictWords);
    Q_ASSERT(!res.second); // sequences
    Q_ASSERT(seqWords.size()>0);

    // dictionary word 'victor'
    res = pa.getPasswordQualityReport( "yhtvictormkg", weight, seqWords, dictWords);
    Q_ASSERT(!res.second); // too short
    Q_ASSERT(dictWords.size()>0);
    Q_ASSERT(dictWords[0] == "victor");

    res = pa.getPasswordQualityReport( "yhtblue866", weight, seqWords, dictWords);
    Q_ASSERT(!res.second); // too short
    Q_ASSERT(dictWords.size()>0);
    Q_ASSERT(dictWords[0] == "blue8");

    // happy path, 1 password examples
    res = pa.getPasswordQualityReport( "VndPZF9foojAcAe", weight, seqWords, dictWords);
    Q_ASSERT(res.second);
    Q_ASSERT(dictWords.size()==1); // foo
    Q_ASSERT(seqWords.size()==0);

    res = pa.getPasswordQualityReport( "ravine-skirt-bar-peltry", weight, seqWords, dictWords);
    Q_ASSERT(res.second);
    Q_ASSERT(dictWords.size()==4); // 4 is correct
    Q_ASSERT(seqWords.size()==0);

    // PIN   965126153077
    res = pa.getPasswordQualityReport( "96512615307736572584", weight, seqWords, dictWords);
    Q_ASSERT(res.second);
    Q_ASSERT(dictWords.size()>0); // there are many small
    Q_ASSERT(seqWords.size()==0);


}

}

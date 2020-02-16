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


#include "testWordSequenser.h"
#include "../util/WordSequences.h"


namespace test {

void testWordSequences() {
    dict::WordSequences seqChecker = dict::buldPasswordChackWordSequences();

    QString str = "abcuUJMq";
    QVector<double>  weights( str.size(), 1.0 );
    QStringList res = seqChecker.detectSequences(str, weights, 1.0 );
    Q_ASSERT( weights[0] == 1.0/3 );
    Q_ASSERT( weights[1] == 1.0/3 );
    Q_ASSERT( weights[2] == 1.0/3 );
    Q_ASSERT( weights[3] == 1.0 );
    Q_ASSERT( weights[4] == 1.0/3 );
    Q_ASSERT( weights[5] == 1.0/3 );
    Q_ASSERT( weights[6] == 1.0/3 );
    Q_ASSERT( weights[7] == 1.0 );
    Q_ASSERT( res.size() == 2 );
    Q_ASSERT( res[0] == "abc" );
    Q_ASSERT( res[1] == "UJM" );

}

}


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

#include "Generator.h"
#include <QTime>

namespace util {

static const QString Base58Alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

// Genrate a secret. Note, random number generator is not cryptography secure because of QT version
QString generateSecret(int length) {
    qsrand( static_cast<quint64>( QTime::currentTime().msecsSinceStartOfDay() ) );
    QString result;
    for ( int t=0;t<length; t++) {
        result.append( Base58Alphabet[qrand()%Base58Alphabet.length()] );
    }

    return result;
}

}

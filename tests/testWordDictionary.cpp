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

#include "testWordDictionary.h"
#include "../util/WordDictionary.h"

namespace test {

void testWordDictionary() {
    dict::WordDictionary dict2(":/resource/passwords-1k.dat");
    dict::WordDictionary dict(":/resource/passwords-10k.dat");

    // Last item 'zzzzzzzz'
    Q_ASSERT(dict.findLongestWord("zzzzzzzzzzz") == "zzzzzzzz");
    Q_ASSERT(dict.findLongestWord("{zzzzzzz") == "");

    // First item: '*****'
    Q_ASSERT(dict.findLongestWord("*****") == "*****");
    Q_ASSERT(dict.findLongestWord("*****234") == "*****");
    Q_ASSERT(dict.findLongestWord("****") == "");
    Q_ASSERT(dict.findLongestWord("(****") == ""); // '(' comes before'*'

    Q_ASSERT(dict2.findLongestWord("victor58476") == "victor");
    Q_ASSERT(dict2.findLongestWord("victor") == "victor");
    Q_ASSERT(dict2.findLongestWord("blue") == "blue");
    Q_ASSERT(dict2.findLongestWord("blue8785") == "blue"); // note we have a keyword blue123 that goes after 'blue'

    Q_ASSERT(dict2.findLongestWord("avictor") == "");
    Q_ASSERT(dict2.findLongestWord("ablue") == "");

}


}

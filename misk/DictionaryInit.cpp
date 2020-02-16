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

#include "DictionaryInit.h"
#include <QSet>
#include "../util/WordDictionary.h"


namespace misk {


static bool compressDictionary( const QString & inFileName, QSet<QString> & processedWords,
                                const QString & outFileName ) {
    QSet<QString> words = dict::readWords( inFileName );
    words -= processedWords;
    processedWords += words;
    QStringList  wordList = dict::convertToStacked( words );

    return dict::compressWords( wordList, outFileName );
}


void provisionDictionary() {

    QSet <QString> processedWords;

/*    compressDictionary("/mw/SecLists-master/Passwords/xato-net-10-million-passwords-100.txt", processedWords,
                       "/mw/mwc-qt-wallet/resource/passwords-100.dat");*/

    compressDictionary("/mw/SecLists-master/Passwords/xato-net-10-million-passwords-1000.txt", processedWords,
                       "/mw/mwc-qt-wallet/resource/passwords-1k.dat");


    compressDictionary("/mw/SecLists-master/Passwords/xato-net-10-million-passwords-10000.txt", processedWords,
                       "/mw/mwc-qt-wallet/resource/passwords-10k.dat");

    compressDictionary("/mw/SecLists-master/Passwords/xato-net-10-million-passwords-100000.txt", processedWords,
                       "/mw/mwc-qt-wallet/resource/passwords-100k.dat");

    compressDictionary("/mw/SecLists-master/Passwords/xato-net-10-million-passwords-1000000.txt", processedWords,
                       "/mw/mwc-qt-wallet/resource/passwords-1M.dat");

/*
     10 M takes 18Mb to store. Too much for us. Also reading will be long as well
     compressDictionary( "/mw/SecLists-master/Passwords/xato-net-10-million-passwords.txt", processedWords,
                        "/mw/mwc-qt-wallet/resource/passwords-10M.dat" );
*/

    dict::WordDictionary dict("/mw/mwc-qt-wallet/resource/passwords-10k.dat");


    // Last item 'zzzzzzzz'
    Q_ASSERT(dict.findLongestWord("zzzzzzzzzzz") == "zzzzzzzz");
    Q_ASSERT(dict.findLongestWord("{zzzzzzz") == "");

    // First item: '*****'
    Q_ASSERT(dict.findLongestWord("*****") == "*****");
    Q_ASSERT(dict.findLongestWord("*****234") == "*****");
    Q_ASSERT(dict.findLongestWord("****") == "");
    Q_ASSERT(dict.findLongestWord("(****") == ""); // '(' comes before'*'

/*    this test is valid only we build 10k dictionary alone
    Q_ASSERT(dict.findLongestWord("victor58476") == "victor");
    Q_ASSERT(dict.findLongestWord("victor") == "victor");
    Q_ASSERT(dict.findLongestWord("blue") == "blue");
    Q_ASSERT(dict.findLongestWord("blue8785") == "blue"); // note we have a keyword blue123 that goes after 'blue'
*/
    Q_ASSERT(dict.findLongestWord("avictor") == "");
    Q_ASSERT(dict.findLongestWord("ablue") == "");
}


}

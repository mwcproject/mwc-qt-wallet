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

#ifndef MWC_QT_WALLET_WORDDICTIONARY_H
#define MWC_QT_WALLET_WORDDICTIONARY_H

#include <QString>
#include <QPair>
#include <QMap>
#include <QStringList>

// Compress file with QT zip-like compressor.
// Expected that the result will be moved into the resources.
// This code is a utility that will never run in the production

namespace dict {

    // Expected lo case inputs
    class WordDictionary {
    private:
        QStringList stackWords;
        QMap<QString, QPair<int, QString> > stackIndex;
    public:
        // Load from file and init
        WordDictionary(QString fileName);

        bool isEmpty() const {return stackWords.isEmpty();}

        // Expected lo case inputs
        QString findLongestWord(const QString & str) const;

        // scan all stirng for dictionary words. If found, the weights will be adjusted
        QStringList detectDictionaryWords( const QString & str, QVector<double> & weights, double seqWeightSum ) const;

    };

    // Special characters used to control the stack...
    const int STACK_CHANGE_MAX = 15;
    const ushort stackPushCh0 = 1; // first symbol for stack push, startign from 1
    const ushort stackPopCh0 = 16; // first symbol for stack pop, starting from 1, MAX 16+15-1= 30

    // read words from the path2read file
    // allWords - set of the word that was processed before. Will be updated with a new w
    // result - resulting set of words
    QSet<QString> readWords( const QString & path2read );

    // Convert set of words into prefixed format
    QStringList convertToStacked( const QSet<QString> & word );

    // Compress the stream and save it into the file.
    bool compressWords( const QStringList & stackedWords, const QString & compressedFn);

    // Read the stream, uncompress it and build the index of the prefixes with the stacked list
    QStringList decompressWords(QString decompressedFn );

    // return: <stack for, <index at stackedWords, stack prev > >
    QMap<QString, QPair<int, QString> > buildStackIndex( const QStringList & stackedWords );

    // Find the words in the dictionary
    // Expected that word starting from the first letter of the string
    // Extected that everyitng is lowercase
    QString findLongestWord( const QString & str, const QStringList & stackedWords,
                  const QMap<QString, QPair<int, QString> > & stackIndex, QString * firstStack = nullptr );

};


#endif //MWC_QT_WALLET_WORDDICTIONARY_H

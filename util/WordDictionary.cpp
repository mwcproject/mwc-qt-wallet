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

#include "WordDictionary.h"
#include "../util/Files.h"
#include <QDir>
#include <QTextStream>
#include <QtGlobal>
#include <QDebug>
#include <QDataStream>

namespace dict {

///////////////////////////////////////////////////////////////////////////
//  Dictionary

// Load from file and init
WordDictionary::WordDictionary(QString fileName) {
    stackWords = decompressWords(fileName );
    stackIndex =  buildStackIndex( stackWords );
}

// Expected lo case inputs
QString WordDictionary::findLongestWord(const QString & str) const {
    QString firstStack;
    QString res = dict::findLongestWord( str, stackWords, stackIndex, &firstStack );
    if (!res.isEmpty())
        return res;

    for (int l=firstStack.length(); l>=1; l-- ) {
        QString res = dict::findLongestWord( str.left(l), stackWords, stackIndex );
        if ( !res.isEmpty() )
            return res;
    }

    return "";
}

// scan all stirng for dictionary words. If found, the weights will be adjusted
QStringList WordDictionary::detectDictionaryWords( const QString & str, QVector<double> & weights, double seqWeightSum ) const {

    QString str2check = str.toLower();

    int idx0 = 0;
    QSet<QString> foundWords;

    while( str2check.length()>=2 ) {
        // checking if there is a word

        QString wrd = findLongestWord(str2check);
        if (wrd.length()>0) {
            // we found something...
            foundWords += str.mid(idx0, wrd.length());

            int idx1 = idx0;
            int idx2 = idx0 + wrd.length();

            double w = seqWeightSum / wrd.size();
            for (int t=idx1; t<idx2; t++)
                weights[t] = std::min( weights[t], w );
        }
        str2check.remove(0,1);
        idx0++;
    }

    return QStringList( foundWords.values() );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------------------------

QStringList readAllWords( QString path ) {

    QStringList result;

    QFileInfo info(path);
    Q_ASSERT(info.exists());

    if ( info.isFile() ) {
        result = util::readTextFile(path);
    }
    else {
        QDir dir(path);

        // scanning subdirs first
        for (QString d : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            result = result + readAllWords(path + "/" + d);
        }

        // scanning files
        for (QString fn : dir.entryList(QDir::Files)) {
            if (!fn.endsWith(".txt"))
                continue;

            result = result + util::readTextFile(path + "/" + fn);
        }
    }

    return result;
}


// read words from the files, filer the same,
// generate all lowcase, all uppercase and dump them
// into resultFN
QSet<QString> readWords(const QString & path2read ) {
    // conver all to lower case. Will do case insensitive look up
    QSet<QString> result;
    for (auto & w : readAllWords(path2read) ) {
        if (w.length()<3)
            continue; // 1 or 2 symbols are fine. It is expected to be at the top list

        QString word = w.toLower();
        bool allLatin = true;
        for (QChar ch : word)
            if ( ch.toLatin1() != ch.unicode() )
                allLatin = false;

        if (allLatin)
            result += w.toLower();
    }
    return result;
}


static void getStackIOs(int & firstChIdx, int & stackPop, int & stackPush,
                        const QString & wrd) {

    firstChIdx = stackPop = stackPush = 0;

    while(wrd[firstChIdx].unicode() <= stackPopCh0+STACK_CHANGE_MAX-1 ) {
        uchar code = wrd[firstChIdx].unicode();
        if ( code < stackPopCh0 ) {
            stackPush += code - stackPushCh0+1;
        }
        else {
            stackPop += code - stackPopCh0+1;
        }
        firstChIdx++;
    }
}


// All args are in/out!!!
// Returns the reconstructed word.
static QString applyStackOp( QString & stack, int & firstChIdx,
                int & stackPop, int & stackPush,
                const QString & wrd) {

    getStackIOs( firstChIdx, stackPop, stackPush, wrd );

    if (firstChIdx==0)
        return stack + wrd;

    Q_ASSERT( firstChIdx > 0 );
    Q_ASSERT( std::max(stackPop, stackPush)>0);

    if (stackPop>0) {
        Q_ASSERT(stackPop <= stack.size());
        stack = stack.left(stack.size() - stackPop);
    }

    QString resWrd = stack + wrd.mid(firstChIdx);

    if (stackPush>0) {
        Q_ASSERT(stackPush <= wrd.size() - firstChIdx);
        stack += wrd.mid(firstChIdx, stackPush);
    }

    return resWrd;
}

QStringList convertToStacked(const QSet<QString> & word) {

    QStringList lines( word.values() );

    lines.sort( Qt::CaseSensitive );

    QStringList originalLines2check = lines;

    QString stack = "";
    int sz = lines.size();

    for ( int i=0; i<sz; i++ ) {
        const QString & ln = lines[i];
        int stackPop = 0;
        QString resLn;

        while (!stack.isEmpty() && !ln.startsWith(stack)) {
            stackPop++;
            stack.remove(stack.size()-1,1);
        }
        while(stackPop>0) {
            int st = std::min(stackPop, STACK_CHANGE_MAX);
            resLn += QChar(stackPopCh0 + st - 1);
            stackPop -= st;
        }

        // calc stack push...
        int stackPush = 0;
        if (i+1<sz) {
            // need 2 lines for stack to push...
            const QString & nextLn = lines[i+1];
            int szLim = std::min( ln.size(), nextLn.size() );
            for (int u=stack.size(); u<szLim; u++ ) {
                if ( ln[u] != nextLn[u] )
                    break;
                stack += ln[u];
                stackPush++;
            }
            int stPush = stackPush;
            while (stPush>0) {
                int st = std::min(stPush, STACK_CHANGE_MAX);
                resLn += QChar(stackPushCh0 + st - 1);
                stPush -= st;
            }
        }

        resLn += ln.mid( stack.size() - stackPush );

        //qDebug() << i << ": " << ln << " => " << stack << " + " << resLn;

        lines[i] = resLn;
    }


    { // Checking decoding...
        QString stack;
        int sz = lines.size();

        for (int i=0;i<sz;i++) {
            const auto & wrd = lines[i];
            int firstChIdx = 0;
            int stackPop = 0;
            int stackPush = 0;

            QString prevStack = stack;
            QString word2check = applyStackOp( stack, firstChIdx, stackPop, stackPush, wrd);
            QString org = originalLines2check[i];
            Q_ASSERT(word2check == originalLines2check[i]);
        }
    }


    return lines;
}


// Compress the stream.
bool compressWords( const QStringList & stackedWords, const QString & compressedFn) {


    QByteArray data2Save;

    {
        QDataStream out(&data2Save, QIODevice::WriteOnly);

        out << int(0x76344);
        out << int(stackedWords.size());
        for (const auto &w : stackedWords)
            out << w;
    }

    QByteArray resData = qCompress(data2Save, 9);

    QFile outFile(compressedFn);
    if (!outFile.open(QIODevice::WriteOnly))
        return false;

    outFile.write(resData);
    outFile.close();

    return true;
}


// Read the stream, uncompress it and build the index of the prefixes with the stacked list
QStringList  decompressWords(QString decompressedFn ) {
    QStringList stackedWords;
    QFile inFile(decompressedFn);
    if (!inFile.open(QIODevice::ReadOnly))
        return stackedWords;

    QByteArray inData = inFile.readAll();
    inData = qUncompress(inData);

    QDataStream in(inData);

    int id;
    int sz;

    in >> id;
    in >> sz;
    if (id!=0x76344 || sz<=0) {
        Q_ASSERT(false); // not expected to have empty dictionary.
        return stackedWords;
    }

    for ( int i=0; i<sz; i++) {
        QString s;
        in >> s;
        stackedWords << s;
    }
    return stackedWords;
}

// return: <stack for, <index at stackedWords, stack prev > >
QMap<QString, QPair<int, QString> > buildStackIndex( const QStringList & stackedWords ) {
    QString stack;
    int sz = stackedWords.size();

    QMap<QString, QPair<int, QString> > stackIndex;

    for (int i=0;i<sz;i++) {
        const auto & wrd = stackedWords[i];
        int firstChIdx = 0;
        int stackPop = 0;
        int stackPush = 0;

        QString prevStack = stack;
        applyStackOp( stack, firstChIdx, stackPop, stackPush, wrd);

        if (stackPush>0) {
            stackIndex[stack] = QPair<int, QString>(i, prevStack);
        }
    }

    return stackIndex;
}

// Find the words in the dictionary
// Expected that word starting from the first letter of the string
// Extected that everyitng is lowercase
QString findLongestWord( const QString & str, const QStringList & stackedWords,
                         const QMap<QString, QPair<int, QString> > & stackIndex, QString * firstStack ) {
    Q_ASSERT(stackedWords.size()>0);
    Q_ASSERT(stackIndex.size()>0);

    int loIdx = 0;
    int hiIdx = stackedWords.size();
    QString loStack;
    if (!stackIndex.isEmpty()) {
        auto loIter = stackIndex.lowerBound(str);
        if (loIter!=stackIndex.end())
            hiIdx = loIter.value().first+1;

        if (loIter != stackIndex.begin()) {
            loIter--;
            loIdx = loIter.value().first;
            loStack = loIter.value().second;
        }
    }
    if (str < loStack) {
        loIdx = 0;
        loStack = "";
    }

    if (firstStack )
        *firstStack = loStack;

    Q_ASSERT(loIdx<=hiIdx);

    // do search for the best fit on the range...
    // Easiest way just to scan. Even words are sirted we have to apply stack first.
    // No stack - no sorting.
    // To apply Stack we need to scan at least once.
    // So scan has to be here, that is why le't do search with a scan. Index range
    // will be very limited
    QString resStr;
    int firstChIdx = 0;
    int stackPop = 0;
    int stackPush = 0;

    QString stack = loStack;

    for (int idx = loIdx; idx<hiIdx; idx++) {
        const auto & wrd = stackedWords[idx];

        QString word = applyStackOp( stack, firstChIdx, stackPop, stackPush, wrd);
        if ( str.startsWith(word) ) {
            if ( word.length() > resStr.size() )
                resStr = word;
        }

    }

    return resStr;
}


};

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

#ifndef MWC_QT_WALLET_WORDSEQUENCES_H
#define MWC_QT_WALLET_WORDSEQUENCES_H

#include <QChar>
#include <QSet>
#include <QMap>
#include <QVector>

namespace dict {

class WordSequences {
private:
    QMap<QChar, QSet<QChar> > sequenceData;
public:
    // every char is a sequence
    void addSequence(QString seqStr);

    bool isInSequence(QChar ch1, QChar ch2) const;

    // str - strign to analize
    // weights - weights to update
    // seqWeightSum - weight for all sequence. will be divided by all chain.
    // return: liest of found sequences
    QStringList detectSequences( const QString & str, QVector<double> & weights, double seqWeightSum ) const;
};

// Build and init sequence instance for password analisys
WordSequences buldPasswordChackWordSequences();

}

#endif //MWC_QT_WALLET_WORDSEQUENCES_H

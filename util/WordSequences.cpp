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

#include "WordSequences.h"

namespace dict {

void WordSequences::addSequence(QString seqStr) {
    int len = seqStr.size();
    for ( int i=1; i<len; i++ ) {
        QChar ch1 = seqStr[i-1];
        QChar ch2 = seqStr[i];
        sequenceData[ch1]+=ch2;
        sequenceData[ch2]+=ch1;
    }

    // Add selfs
    for (QChar ch : seqStr)
        sequenceData[ch]+=ch;
}


bool WordSequences::isInSequence(QChar ch1, QChar ch2) const {
    return sequenceData[ch1].contains(ch2);
}

static QString applyFoundSeqRange(const QString & str, int idx1, int idx2, QVector<double> & weights, double seqWeightSum) {
    Q_ASSERT(idx2-idx1>=2);
    double w = seqWeightSum / (idx2-idx1);
    for ( int i=idx1; i<idx2; i++ ) {
        weights[i] = std::min( weights[i], w );
    }
    return str.mid(idx1, idx2-idx1);
}

// str - strign to analize
// weights - weights to update
// seqWeightSum - weight for all sequence. will be divided by all chain.
QStringList WordSequences::detectSequences( const QString & str, QVector<double> & weights, double seqWeightSum ) const {
    QStringList result;
    int len = str.size();
    int startSeqIdx = -1;
    for ( int i=1; i<len; i++ ) {
        if ( isInSequence( str[i-1], str[i] ) ) {
            if (startSeqIdx<0)
                startSeqIdx = i-1;
        }
        else {
            if ( startSeqIdx >=0 ) {
                // closing sequence
                result += applyFoundSeqRange(str, startSeqIdx, i, weights, seqWeightSum);
                startSeqIdx = -1;
            }
        }
    }

    if (startSeqIdx>=0) {
        result += applyFoundSeqRange(str, startSeqIdx, len, weights, seqWeightSum);
        startSeqIdx = -1;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

WordSequences buldPasswordChackWordSequences() {
    WordSequences seq;

    seq.addSequence("01234567890");
    seq.addSequence("~!@#$%^&*()_+");
    // alphabet
    seq.addSequence("abcdefghijklmnopqrstuwxyz");
    seq.addSequence("ABCDEFGHIJKLMNOPQRSTUWXYZ");

    // Keyboard
    seq.addSequence("qwertyuiop[]\\");
    seq.addSequence("asdfghjkl;'");
    seq.addSequence("zxcvbnm,./");

    seq.addSequence("QWERTYUIOP{}|");
    seq.addSequence("ASDFGHJKL:");
    seq.addSequence("ZXCVBNM<>?");

    // right vert slices
    seq.addSequence("1qaz");
    seq.addSequence("!QAZ");
    seq.addSequence("2wsx");
    seq.addSequence("@WSX");
    seq.addSequence("3edc");
    seq.addSequence("#EDC");
    seq.addSequence("4rfv");
    seq.addSequence("$RFV");
    seq.addSequence("5tgb");
    seq.addSequence("%TGB");
    seq.addSequence("6yhn");
    seq.addSequence("^YHN");
    seq.addSequence("7ujm");
    seq.addSequence("&UJM");
    seq.addSequence("8ik,");
    seq.addSequence("*IK<");
    seq.addSequence("9ol.");
    seq.addSequence("(OL>");
    seq.addSequence("0p;/");
    seq.addSequence(")P:?");
    seq.addSequence("-['");
    seq.addSequence("_{\"");
    // left vert slices
    seq.addSequence("]'/");
    seq.addSequence("}\"?");
    seq.addSequence("=[;.");
    seq.addSequence("+{:>");
    seq.addSequence("-pl,");
    seq.addSequence("_PL<");
    seq.addSequence("0okm");
    seq.addSequence(")OKM");
    seq.addSequence("9ijn");
    seq.addSequence("(IJN");
    seq.addSequence("8uhb");
    seq.addSequence("*UHB");
    seq.addSequence("7ygv");
    seq.addSequence("&YGV");
    seq.addSequence("6tfc");
    seq.addSequence("^TFC");
    seq.addSequence("5rdx");
    seq.addSequence("%RDX");
    seq.addSequence("4esz");
    seq.addSequence("$ESZ");
    seq.addSequence("3wa");
    seq.addSequence("#WA");

    return seq;
}


}


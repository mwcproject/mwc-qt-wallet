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

#ifndef PASSWORDANALYSER_H
#define PASSWORDANALYSER_H

#include <QString>
#include "../util/WordSequences.h"
#include "../util/WordDictionary.h"

namespace util {

// Password analyser is a heavy object
class PasswordAnalyser
{
public:
    PasswordAnalyser(QString attentinColor = "#CCFF33", QString happyColor = "#CCCCCC");
    ~PasswordAnalyser();

    // return html string with a success flag that describe the quality of the password
    // Example: <font color=#CCFF33>aaaa</font>
    QPair<QString, bool> getPasswordQualityReport(const QString & pass, // in
                                        QVector<double> & weight,
                                        QStringList & seqWords,
                                        QStringList & dictWords);

private:
    static const int PASS_MIN_LEN   = 10;
    constexpr static const double PASS_MIN_WEIGHT = 8.0;
    static const int DICTS_NUM      = 4;

    QString attentinColor;
    QString happyColor;

    dict::WordDictionary * dictionaries[DICTS_NUM];
    double dictionaryWeight[DICTS_NUM];

    dict::WordSequences sequenceAnalyzer;
};

}

#endif // PASSWORDANALYSER_H

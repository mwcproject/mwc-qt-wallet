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

#ifndef TESTSEEDTASK_H
#define TESTSEEDTASK_H

#include <QString>

namespace core {

class TestSeedTask
{
public:
    TestSeedTask();
    TestSeedTask(int wordIndex,
        QString word);

    TestSeedTask(const TestSeedTask & ) = default;
    TestSeedTask & operator = (const TestSeedTask & ) = default;

    int getWordIndex() const {return wordIndex;}

    // return true if test results are correct. So this test is passed
    bool applyInputResults(QString word);
private:
    int wordIndex;
    QString word;
};

// startIdx & N are applicable for mobile wallet only. For mobile wallet we are requesting the subset of the words
// In case of error, all words are regenerated
QVector<core::TestSeedTask> generateSeedTasks( const QVector<QString> & seed, int startIdx, int N );

}

#endif // TESTSEEDTASK_H

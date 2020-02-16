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

#include "testseedtask.h"
#include <QVector>

#ifdef Q_OS_WIN
#include <time.h>
#endif

namespace core {

TestSeedTask::TestSeedTask() :
    wordIndex(-1),
    failures(0)
{
}

TestSeedTask::TestSeedTask(int _wordIndex, QString _word) :
    wordIndex(_wordIndex),
    word(_word),
    failures(0)
{
}

bool TestSeedTask::applyInputResults(QString w) {
    if (word == w) {
        failures = 0;
        return true;
    }
    failures++;
    return false;
}


QVector<core::TestSeedTask> generateSeedTasks( const QVector<QString> & seed ) {

    // Generate tasks. Need to review the words one by one
    QVector< QPair<int,QString> > words;
    QVector<core::TestSeedTask> confirmTasks;
    for ( int i=0;i<seed.size();i++ ) {
        words.push_back(QPair<int, QString>(i + 1, seed[i]));
        confirmTasks.push_back( core::TestSeedTask( i + 1, seed[i]) );
    }

/* Tasks will go in random order
   srand (time(nullptr));

    while( words.size()>0 ) {
        int idx = rand() % words.size();
        confirmTasks.push_back( core::TestSeedTask( words[idx].first, words[idx].second) );
        words.remove(idx);
    }*/

    return confirmTasks;
}


}

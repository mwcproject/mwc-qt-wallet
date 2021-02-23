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
#include <QSet>
#include <ctime>
#include <cstdlib>

#ifdef Q_OS_WIN
#include <time.h>
#endif

namespace core {

TestSeedTask::TestSeedTask() :
    wordIndex(-1)
{
}

TestSeedTask::TestSeedTask(int _wordIndex, QString _word) :
    wordIndex(_wordIndex),
    word(_word)
{
}

bool TestSeedTask::applyInputResults(QString w) {
    if (word == w) {
        return true;
    }
    return false;
}

QVector<core::TestSeedTask> generateSeedTasks( const QVector<QString> & seed, int startIdx, int N ) {
    // Generate tasks. Need to review the words one by one
    QVector<core::TestSeedTask> confirmTasks;
#ifdef WALLET_DESKTOP
    for ( int i=0;i<seed.size();i++ ) {
        confirmTasks.push_back( core::TestSeedTask( i + 1, seed[i]) );
    }
#endif

#ifdef WALLET_MOBILE
    // For mobile we will generate 6 random words to confirm.
    srand (time(nullptr));
    QSet<int> idxs;
    while( confirmTasks.size()<N ) {
        int wrdIdx = rand() % seed.size();
        if ( wrdIdx<startIdx || idxs.contains(wrdIdx))
            continue;
        idxs += wrdIdx;
        confirmTasks.push_back( core::TestSeedTask( wrdIdx + 1, seed[wrdIdx]) );
    }
    std::sort(confirmTasks.begin(), confirmTasks.end(), [](const core::TestSeedTask &c1, const core::TestSeedTask &c2) { return c1.getWordIndex() < c2.getWordIndex(); } );
#endif

/* Tasks will go in random order
   srand (time(nullptr));

    while( words.size()>0 ) {
        int idx = rand() % words.size();
        confirmTasks.push_back( core::TestSeedTask( words[idx].first, words[idx].second) );
        words.remove(idx);
    }*/

    Q_ASSERT(!confirmTasks.isEmpty());
    return confirmTasks;
}


}

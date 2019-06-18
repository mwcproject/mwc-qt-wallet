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
    for ( int i=0;i<seed.size();i++ )
        words.push_back( QPair<int,QString>(i+1, seed[i]) );

    QVector<core::TestSeedTask> confirmTasks;

    srand (time(nullptr));

    while( words.size()>0 ) {
        int idx = rand() % words.size();
        confirmTasks.push_back( core::TestSeedTask( words[idx].first, words[idx].second) );
        words.remove(idx);
    }
    return confirmTasks;
}


}

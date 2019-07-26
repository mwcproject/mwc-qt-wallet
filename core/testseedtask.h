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

    bool isTestCompletelyFailed() const {return failures>=3;}
private:
    int wordIndex;
    QString word;
    int failures;
};

QVector<core::TestSeedTask> generateSeedTasks( const QVector<QString> & seed );

}

#endif // TESTSEEDTASK_H

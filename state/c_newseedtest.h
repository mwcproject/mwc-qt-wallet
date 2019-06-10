#ifndef NEWSEEDTEST_H
#define NEWSEEDTEST_H

#include "state.h"
#include <QString>
#include <QPair>
#include "../core/testseedtask.h"

namespace state {

class NewSeedTest : public State
{
public:
    NewSeedTest(const StateContext & context);
    virtual ~NewSeedTest() override;

    void submit(QString word);

protected:
    virtual NextStateRespond execute() override;
private:
    core::TestSeedTask currentTask;
};

}

#endif // NEWSEEDTEST_H

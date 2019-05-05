#ifndef NEWSEEDTEST_H
#define NEWSEEDTEST_H

#include "state.h"
#include <QString>
#include <QPair>

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
    QPair<int,QString> currentTask;
};

}

#endif // NEWSEEDTEST_H

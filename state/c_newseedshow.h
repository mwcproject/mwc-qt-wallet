#ifndef NEWSEEDSHOW_H
#define NEWSEEDSHOW_H


#include "state.h"

namespace state {

class SubmitCaller {
public:
    virtual void submit() = 0;
};

class NewSeedShow : public State, public SubmitCaller
{
public:
    NewSeedShow(const StateContext & context);
    virtual ~NewSeedShow() override;

    virtual void submit() override;
protected:
    virtual NextStateRespond execute() override;
};

}

#endif // NEWSEEDSHOW_H

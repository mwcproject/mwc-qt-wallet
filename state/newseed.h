#ifndef NEWSEED_H
#define NEWSEED_H

#include "state.h"

namespace state {


class NewSeed : public State
{
public:
    NewSeed(const StateContext & context);
    virtual ~NewSeed() override;

    void submit();
protected:
    virtual NextStateRespond execute() override;
};

}

#endif // NEWSEED_H

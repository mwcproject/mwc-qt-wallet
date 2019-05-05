#ifndef NEWSEEDSHOW_H
#define NEWSEEDSHOW_H


#include "state.h"

namespace state {

class NewSeedShow : public State
{
public:
    NewSeedShow(const StateContext & context);
    virtual ~NewSeedShow() override;

    void submit();
protected:
    virtual NextStateRespond execute() override;
};

}

#endif // NEWSEEDSHOW_H

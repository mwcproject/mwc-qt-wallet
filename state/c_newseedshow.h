#ifndef NEWSEEDSHOW_H
#define NEWSEEDSHOW_H


#include "state.h"

namespace wnd {
    class NewSeed;
}

namespace state {

class SubmitCaller {
public:
    virtual void submit() = 0;
    virtual void wndDeleted(wnd::NewSeed * w) = 0;
};

class NewSeedShow : public State, public SubmitCaller
{
public:
    NewSeedShow( StateContext * context);
    virtual ~NewSeedShow() override;

    virtual void submit() override;
    virtual void wndDeleted(wnd::NewSeed * w) override { if(w==wnd) wnd = nullptr; }
protected:
    virtual NextStateRespond execute() override;
private:
    wnd::NewSeed * wnd = nullptr;
};

}

#endif // NEWSEEDSHOW_H

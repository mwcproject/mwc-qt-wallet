#ifndef CONNECT2NODE_H
#define CONNECT2NODE_H

#include "state.h"
#include "../wallet/wallet.h"


namespace state {

class Connect2Node : public State
{
public:
    enum CONNECT_CHOICE {NODE_LOCAL, NODE_POOL, NODE_MANUAL};

    Connect2Node(const StateContext & context);
    virtual ~Connect2Node() override;

    void applyChoice(CONNECT_CHOICE connect);
    void cancel();

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // CONNECT2NODE_H

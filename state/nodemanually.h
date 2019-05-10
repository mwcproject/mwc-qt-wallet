#ifndef NODEMANUALLY_H
#define NODEMANUALLY_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {

class NodeManually : public State
{
public:
    NodeManually(const StateContext & context);
    virtual ~NodeManually() override;

    QPair<bool,QString> testMwcNode(QString host, int port);
    void submitMwcNode(QString host, int port);
    void cancel();

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // NODEMANUALLY_H

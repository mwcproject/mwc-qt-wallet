#ifndef NODESTATUS_H
#define NODESTATUS_H

#include "state.h"
#include "../wallet/wallet.h"


namespace state {

class NodeStatus : public State
{
public:
    NodeStatus(const StateContext & context);
    virtual ~NodeStatus() override;

    wallet::WalletConfig getWalletConfig();
    wallet::NodeStatus getNodeStatus();

    void requestToChangeTheNode();
protected:
    virtual NextStateRespond execute() override;
};

}

#endif // NODESTATUS_H

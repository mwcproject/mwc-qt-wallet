#ifndef RECEIVECOINS_H
#define RECEIVECOINS_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {

class Transactions;
class Listening;
class FileTransactions;


class ReceiveCoins : public State
{
public:
    ReceiveCoins(const StateContext & context);
    virtual ~ReceiveCoins() override;

protected:
    virtual NextStateRespond execute() override;
private:
    // ReceiveCoins accumulate features from 3 different states.
    // That is why we can delegate functionality to them
    Transactions * transactionsState;
    Listening * listeningState;
    //FileTransactions * fileTransactionsState;
};

}

#endif // RECEIVECOINS_H

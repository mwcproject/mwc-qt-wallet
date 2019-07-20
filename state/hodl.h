#ifndef HODL_H
#define HODL_H


#include "state.h"
#include "../wallet/wallet.h"
#include <QVector>

namespace state {

class Hodl : public State
{
public:
    Hodl(StateContext * context);
    virtual ~Hodl() override;

    QVector<wallet::WalletTransaction> getTransactions();

    void submitForHodl( const QVector<QString> & transactions );

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // HODL_H

#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {


class Outputs : public State
{
public:
    Outputs(const StateContext & context);
    virtual ~Outputs() override;

    // Current transactions that wallet has
    QVector<wallet::WalletOutput> getOutputs();

    QString getCurrentAccountName() const;

    // IO for columns widhts
    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // OUTPUTS_H

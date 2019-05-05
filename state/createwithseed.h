#ifndef CREATEWITHSEED_H
#define CREATEWITHSEED_H

#include "state.h"
#include <QString>
#include <QPair>

namespace state {


class CreateWithSeed: public State
{
public:
    CreateWithSeed(const StateContext & context);
    virtual ~CreateWithSeed() override;

    QPair<bool, QString> createWalletWithSeed( QVector<QString> seed );

protected:
    virtual NextStateRespond execute() override;

};


}

#endif // CREATEWITHSEED_H

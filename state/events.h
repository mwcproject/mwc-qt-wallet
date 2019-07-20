#ifndef EVENTS_H
#define EVENTS_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QVector>

namespace state {

class Events : public State
{
public:
    Events( StateContext * context );
    virtual ~Events() override;

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // EVENTS_H

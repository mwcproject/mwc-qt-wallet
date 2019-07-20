#ifndef INITACCOUNT_H
#define INITACCOUNT_H

#include "state.h"
#include <QString>

namespace state {

class InitAccount : public State
{
public:
    InitAccount(StateContext * context);
    virtual ~InitAccount() override;

    void setPassword(const QString & password);

protected:
    virtual NextStateRespond execute() override;
};


}


#endif // INITACCOUNT_H

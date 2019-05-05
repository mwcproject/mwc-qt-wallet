#ifndef INPUTPASSWORD_H
#define INPUTPASSWORD_H

#include "state.h"
#include <QString>

namespace state {

class InputPassword : public State
{
public:
    InputPassword(const StateContext & context);
    virtual ~InputPassword() override;

    bool checkPassword(const QString & password);

    void submitPassword(const QString & password);

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // INPUTPASSWORD_H

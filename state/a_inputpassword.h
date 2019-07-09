#ifndef INPUTPASSWORD_H
#define INPUTPASSWORD_H

#include "state.h"
#include <QObject>
#include "../wallet/wallet.h"

namespace wnd {
    class InputPassword;
}

namespace state {

class InputPassword : public QObject, public State
{
    Q_OBJECT
public:
    InputPassword(const StateContext & context);
    virtual ~InputPassword() override;

    void deleteWnd() {wnd = nullptr;}

    // Async call to submit the password. This state migth get back to UI if password is incorrect
    void submitPassword(const QString & password);

protected:
    virtual NextStateRespond execute() override;

protected slots:
    void onInitWalletStatus( wallet::InitWalletStatus  status );
    void onWalletBalanceUpdated();

private:
    wnd::InputPassword * wnd = nullptr;
};

}

#endif // INPUTPASSWORD_H

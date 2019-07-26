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
    InputPassword(StateContext * context);
    virtual ~InputPassword() override;

    void deleteWnd(wnd::InputPassword * w) { if (w==wnd) wnd = nullptr;}

    // Async call to submit the password. This state migth get back to UI if password is incorrect
    void submitPassword(const QString & password);

    QPair<bool,bool> getWalletListeningStatus();

protected:
    virtual NextStateRespond execute() override;

protected slots:
    // Result of the login
    void onLoginResult(bool ok);

    void onWalletBalanceUpdated();

    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);

private:
    wnd::InputPassword * wnd = nullptr;
    bool inLockMode = false;
};

}

#endif // INPUTPASSWORD_H

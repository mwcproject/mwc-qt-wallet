#ifndef PASSWORDFORSEED_H
#define PASSWORDFORSEED_H

#include "../wnd_core/walletwindow.h"

namespace Ui {
class PasswordForSeed;
}

class SessionPassword;

class PasswordForSeed : public WalletWindow
{
    Q_OBJECT

public:
    explicit PasswordForSeed(WalletWindowType appType, QWidget *parent, SessionPassword * pswdData);
    virtual ~PasswordForSeed() override;

    bool validateData() override;

private slots:
    void on_password1Edit_textChanged(const QString &arg1);

private:
    Ui::PasswordForSeed *ui;
    SessionPassword * pswd;
};

#endif // PASSWORDFORSEED_H

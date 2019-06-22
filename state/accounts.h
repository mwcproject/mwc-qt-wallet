#ifndef ACCOUNTSS_H
#define ACCOUNTSS_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QVector>

namespace state {


class Accounts : public State
{
public:
    Accounts(const StateContext & context);
    virtual ~Accounts() override;

    // get balance for current account
    QVector<wallet::AccountInfo> getWalletInfo();

    void resync();

    // Make this account current
    void switchAccount(QString account);

    // add new account
    void createAccount(QString account);
protected:
    virtual NextStateRespond execute() override;
};

}

#endif // ACCOUNTSS_H

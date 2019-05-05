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
    wallet::WalletInfo getWalletInfo();

    // resync (update) account
    wallet::WalletInfo resync();

    // Get account list
    QVector<QString> getAccounts();

    // Make this account current
    QPair<bool, QString> activateAccount(QString account);

    // add new account
    QPair<bool, QString> addAccount(QString account);
protected:
    virtual NextStateRespond execute() override;
};

}

#endif // ACCOUNTSS_H

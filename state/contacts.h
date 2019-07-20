#ifndef CONTACTS_H
#define CONTACTS_H


#include "state.h"
#include "../wallet/wallet.h"

namespace state {


class Contacts : public State
{
public:
    Contacts(StateContext * context);
    virtual ~Contacts() override;

    // Get the contacts
    QVector<wallet::WalletContact> getContacts();

    // Add new contact
    QPair<bool, QString> addContact( const wallet::WalletContact & contact );

    // Remove contact. return false if not found
    QPair<bool, QString> deleteContact( const QString & name );

    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & width);

protected:
    virtual NextStateRespond execute() override;

};

}

#endif // CONTACTS_H

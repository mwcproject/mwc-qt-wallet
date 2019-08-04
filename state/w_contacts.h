#ifndef CONTACTS_H
#define CONTACTS_H


#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace state {


class Contacts : public State
{
public:
    Contacts(StateContext * context);
    virtual ~Contacts() override;

    // Get the contacts
    QVector<core::ContactRecord> getContacts();

    QPair<bool, QString> addContact( const core::ContactRecord & contact );
    QPair<bool, QString> deleteContact( const core::ContactRecord & contact );
    QPair<bool, QString> updateContact( const core::ContactRecord & prevValue, const core::ContactRecord & newValue );

    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & width);

protected:
    virtual NextStateRespond execute() override;

};

}

#endif // CONTACTS_H

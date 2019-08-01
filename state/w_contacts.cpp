#include "w_contacts.h"
#include "windows/w_contacts_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

Contacts::Contacts(StateContext * context) :
    State(context, STATE::CONTACTS)
{

}

Contacts::~Contacts() {}

NextStateRespond Contacts::execute() {
    if (context->appContext->getActiveWndState() != STATE::CONTACTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context->wndManager->switchToWindowEx(
                new wnd::Contacts( context->wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

QVector<wallet::WalletContact> Contacts::getContacts() {
    return context->wallet->getContacts();
}

QPair<bool, QString> Contacts::addContact( const wallet::WalletContact & contact ) {
    return context->wallet->addContact(contact);
}

QPair<bool, QString> Contacts::deleteContact( const QString & name ) {
    return context->wallet->deleteContact(name);
}

QVector<int> Contacts::getColumnsWidhts() const {
    return context->appContext->getIntVectorFor("ContactsTblWidth");
}

void Contacts::updateColumnsWidhts(const QVector<int> & width) {
    context->appContext->updateIntVectorFor("ContactsTblWidth", width);
}


}


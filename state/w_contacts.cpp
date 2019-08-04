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

QVector<core::ContactRecord> Contacts::getContacts() {
    return context->appContext->getContacts();
}

QPair<bool, QString> Contacts::addContact( const core::ContactRecord & contact ) {
    return context->appContext->addContact(contact);
}

QPair<bool, QString> Contacts::deleteContact( const core::ContactRecord & contact ) {
    return context->appContext->deleteContact(contact);
}

QPair<bool, QString> Contacts::updateContact( const core::ContactRecord & prevValue, const core::ContactRecord & newValue ) {
    return context->appContext->updateContact( prevValue, newValue );
}

QVector<int> Contacts::getColumnsWidhts() const {
    return context->appContext->getIntVectorFor("ContactsTblWidth");
}

void Contacts::updateColumnsWidhts(const QVector<int> & width) {
    context->appContext->updateIntVectorFor("ContactsTblWidth", width);
}


}


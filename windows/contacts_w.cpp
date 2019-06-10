#include "contacts_w.h"
#include "ui_contacts.h"
#include "../state/contacts.h"
#include "contacteditdlg.h"
#include "../control/messagebox.h"

namespace wnd {

Contacts::Contacts(QWidget *parent, state::Contacts * _state) :
    QWidget(parent),
    ui(new Ui::Contacts),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Contacts");

    initTableHeaders();
    updateContactTable();

    ui->contactsTable->setFocus();
}

Contacts::~Contacts()
{
    saveTableHeaders();
    delete ui;
}

void Contacts::updateButtons() {
    int idx = getSelectedContactIndex();

    ui->addButton->setEnabled(true);
    ui->editButton->setEnabled(idx>=0);
    ui->removeButton->setEnabled(idx>=0);
}


void Contacts::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 3 ) {
        widths = QVector<int>{30,100,300};
    }
    Q_ASSERT( widths.size() == 3 );
    ui->contactsTable->setColumnWidths( widths );
}

void Contacts::saveTableHeaders() {
    state->updateColumnsWidhts( ui->contactsTable->getColumnWidths() );
}

void Contacts::updateContactTable() {
    contacts = state->getContacts();

    ui->contactsTable->clearData();
    int idx = 0;
    for ( const wallet::WalletContact & cont : contacts ) {
        ui->contactsTable->appendRow( QVector<QString>{
                                          QString::number(++idx),
                                          cont.name,
                                          cont.address
                                      } );
    }
}

int  Contacts::getSelectedContactIndex() const {
    QList<QTableWidgetItem*> selList = ui->contactsTable->selectedItems();
    if (selList.size()==0)
        return -1;

    int idx = selList[0]->row();
    if (idx>=0 && idx<contacts.size())
        return idx;

    return -1;
}


void Contacts::on_addButton_clicked()
{
    ContactEditDlg dlg(this, wallet::WalletContact(),
                                   contacts, false );
    if (dlg.exec() == QDialog::Accepted) {
        QPair<bool, QString> res = state->addContact(dlg.getContact());

        if (!res.first) {
            control::MessageBox::message(this, "Error", "You contact wasn't added because of the error: " + res.second);
        }

        updateContactTable();
    }

}

void Contacts::on_editButton_clicked()
{
    int idx = getSelectedContactIndex();
    if (idx<0) // expected to be disabled
        return;

    ContactEditDlg dlg(this, contacts[idx],
                                   contacts, true );
    if (dlg.exec() == QDialog::Accepted) {
        auto contact = dlg.getContact();

        // Update is not atomic. There is a chnce to lost the contact.
        QPair<bool, QString> res = state->deleteContact(contact.name);
        if (res.first)
            res = state->addContact(contact);

        if (!res.first)
            control::MessageBox::message(this, "Error", "Unable to update the contact data. Error: " + res.second);

        updateContactTable();
    }

}

void Contacts::on_removeButton_clicked()
{
    int idx = getSelectedContactIndex();
    if (idx<0) // expected to be disabled
        return;

    QString name2del = contacts[idx].name;

    if (control::MessageBox::question(this, "Remove a contact", "Remove the selected contact for " + name2del +
                              "? Press 'Yes' to delete.", "Yes", "No", false,true) == control::MessageBox::BTN1 ) {
        QPair<bool, QString> res = state->deleteContact(name2del);
        if (!res.first) {
            control::MessageBox::message(this, "Error", "Unable to remove the contact for "+name2del+".\nError: " + res.second);
        }

        updateContactTable();
    }
}

}


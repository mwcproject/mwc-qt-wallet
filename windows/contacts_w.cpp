#include "contacts_w.h"
#include "ui_contacts.h"
#include "../state/contacts.h"
#include "contacteditdlg.h"
#include <QMessageBox>

namespace wnd {

Contacts::Contacts(QWidget *parent, state::Contacts * _state) :
    QWidget(parent),
    ui(new Ui::Contacts),
    state(_state)
{
    ui->setupUi(this);
    initTableHeaders();
    updateContactTable();

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
    ui->contactsTable->setShowGrid(false);

    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 3 ) {
        widths = QVector<int>{20,50,150};
    }
    Q_ASSERT( widths.size() == 3 );
    Q_ASSERT( ui->contactsTable->columnCount() == widths.size() );

    for (int u=0;u<widths.size();u++)
        ui->contactsTable->setColumnWidth(u,widths[u]);
}

void Contacts::saveTableHeaders() {
    int cc = ui->contactsTable->columnCount();
    QVector<int> widths( cc );
    for (int t=0;t<cc;t++)
        widths[t] = ui->contactsTable->columnWidth(t);

    state->updateColumnsWidhts(widths);
}

void Contacts::updateContactTable() {
    contacts = state->getContacts();
    int rowNum = contacts.size();

    QTableWidget * tt = ui->contactsTable;

    tt->clearContents();
    tt->setRowCount(rowNum);

    Q_ASSERT( tt->columnCount() == 3 );
    tt->setSortingEnabled(false);

    for ( int i=0; i<rowNum; i++ ) {
        auto & cont = contacts[i];

        tt->setItem( i, 0, new QTableWidgetItem(QString::number(i+1) ));
        tt->setItem( i, 1, new QTableWidgetItem( cont.name ));
        tt->setItem( i, 2, new QTableWidgetItem( cont.address ));
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
            QMessageBox::critical(this, "Add contact", "You contact wasn't added because of the error: " + res.second);
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
            QMessageBox::critical(this, "Edit contact", "Unable to update the contact data. Error: " + res.second);

        updateContactTable();
    }

}

void Contacts::on_removeButton_clicked()
{
    int idx = getSelectedContactIndex();
    if (idx<0) // expected to be disabled
        return;

    QString name2del = contacts[idx].name;

    if (QMessageBox::question(this, "Remove a contact", "Remove the selected contact for " + name2del +
                              "? Press 'Yes' to delete.") == QMessageBox::Yes ) {
        QPair<bool, QString> res = state->deleteContact(name2del);
        if (!res.first) {
            QMessageBox::critical(this, "Remove contact", "Unable to remove the contact for "+name2del+". Error: " + res.second);
        }

        updateContactTable();
    }
}

}


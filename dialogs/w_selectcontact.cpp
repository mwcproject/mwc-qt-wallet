// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "w_selectcontact.h"
#include "ui_w_selectcontact.h"
#include "../util/stringutils.h"
#include <QListWidgetItem>
#include "../control/messagebox.h"
#include "../state/w_contacts.h"

namespace dlg {

SelectContact::SelectContact(QWidget *parent, state::Contacts * _state ) :
    control::MwcDialog(parent),
    ui(new Ui::SelectContact),
    state(_state)
{
    ui->setupUi(this);

    initTableHeaders();
    updateContactTable("");
}

SelectContact::~SelectContact()
{
    delete ui;
}

void SelectContact::on_cancelButton_clicked()
{
    reject();
}

void SelectContact::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 3 ) {
        widths = QVector<int>{35,150,400};
    }
    Q_ASSERT( widths.size() == 3 );
    ui->contactsTable->setColumnWidths( widths );
}

void SelectContact::saveTableHeaders() {
    state->updateColumnsWidhts( ui->contactsTable->getColumnWidths() );
}

void SelectContact::updateContactTable(const QString & searchStr) {
    contacts.clear();

    QVector<core::ContactRecord> contAll = state->getContacts();

    ui->contactsTable->clearData();
    for ( const auto & cont : contAll ) {
        if ( searchStr.isEmpty() || cont.name.contains( searchStr ) ) {
            ui->contactsTable->appendRow(QVector<QString>{
                    QString::number( contacts.size()),
                    cont.name,
                    cont.address
            });
            contacts.push_back(cont);
        }
    }
}

int  SelectContact::getSelectedContactIndex() const {
    QList<QTableWidgetItem*> selList = ui->contactsTable->selectedItems();
    if (selList.size()==0)
        return -1;

    int idx = selList[0]->row();
    if (idx>=0 && idx<contacts.size())
        return idx;

    return -1;
}

void SelectContact::on_lineEdit_textChanged(const QString &str)
{
    updateContactTable(str);
}

void SelectContact::on_selectButton_clicked()
{
    int idx = SelectContact::getSelectedContactIndex();

    if ( idx<0 || idx>=contacts.size() ) {
        control::MessageBox::messageText(this, "Need info", "Please select a contact that you are going to use");
        return;
    }

    selectedContact = contacts[idx];
    accept();
}

void SelectContact::on_contactsTable_itemDoubleClicked(QTableWidgetItem *item)
{
    Q_UNUSED(item);
    on_selectButton_clicked();
}

}




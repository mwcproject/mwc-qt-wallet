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

#ifndef SELECTCONTACT_H
#define SELECTCONTACT_H

#include <QDialog>
#include  "../wallet/wallet.h"
#include  "../core/appcontext.h"
#include  "../control/mwcdialog.h"

namespace Ui {
class SelectContact;
}

namespace state {
class Contacts;
}

class QTableWidgetItem;

namespace dlg {

class SelectContact : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit SelectContact(QWidget *parent, state::Contacts * state );
    virtual ~SelectContact() override;

    core::ContactRecord getSelectedContact() const {return selectedContact;}

private slots:
    void on_cancelButton_clicked();

    void on_selectButton_clicked();

    void on_contactsTable_itemDoubleClicked(QTableWidgetItem *item);

    void on_deleteButton_clicked();

    void on_addButton_clicked();

    void on_editButton_clicked();

    void on_contactsTable_itemSelectionChanged();

    void on_searchStr_textEdited(const QString & str);

private:
    void initTableHeaders();
    void saveTableHeaders();
    void updateContactTable(const QString & searchStr);

    int  getSelectedContactIndex() const;

    void updateButtons();

private:
    Ui::SelectContact *ui;
    core::ContactRecord selectedContact;
    state::Contacts * state;
    QVector<core::ContactRecord> contacts; // shown contacts
};

}


#endif // SELECTCONTACT_H

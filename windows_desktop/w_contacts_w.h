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

#ifndef CONTACTSW_H
#define CONTACTSW_H

#include "../core_desktop/navwnd.h"
#include "../core/appcontext.h"

namespace Ui {
class Contacts;
}

namespace bridge {
class Config;
}

namespace wnd {

class Contacts : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Contacts(QWidget *parent);
    ~Contacts();

private slots:
    void on_addButton_clicked();
    void on_editButton_clicked();
    void on_deleteButton_clicked();
    void on_contactsTable_cellDoubleClicked(int row, int column);
    void on_contactsTable_itemSelectionChanged();
private:
    void updateButtons();
    void initTableHeaders();

    void updateContactTable();
    void saveTableHeaders();

    // return -1 if nothing selected
    int  getSelectedContactIndex() const;
private:
    Ui::Contacts *ui;
    bridge::Config * config = nullptr;
    QVector<core::ContactRecord> contacts; // Pair of values: [name, address]
};

}

#endif // CONTACTSW_H

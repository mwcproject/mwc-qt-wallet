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

#ifndef CONTACTEDITDLG_H
#define CONTACTEDITDLG_H

#include <QDialog>
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../control/mwcdialog.h"

namespace Ui {
class ContactEditDlg;
}

namespace dlg {

class ContactEditDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit ContactEditDlg(QWidget *parent, const core::ContactRecord & contact,
                            const QVector<core::ContactRecord> & contacts, bool editMode );
    virtual ~ContactEditDlg() override;

    const core::ContactRecord & getContact() const {return contact;}

private slots:
    void on_applyButton_clicked();
    void on_cancelButton_clicked();
private:
    Ui::ContactEditDlg *ui;
    core::ContactRecord contact;
    const QVector<core::ContactRecord> contactList;
};

}

#endif // CONTACTEDITDLG_H

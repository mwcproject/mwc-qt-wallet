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

    void on_lineEdit_textChanged(const QString &str);

    void on_contactsTable_itemDoubleClicked(QTableWidgetItem *item);

private:
    void initTableHeaders();
    void saveTableHeaders();
    void updateContactTable(const QString & searchStr);

    int  getSelectedContactIndex() const;

private:
    Ui::SelectContact *ui;
    core::ContactRecord selectedContact;
    state::Contacts * state;
    QVector<core::ContactRecord> contacts; // shown contacts
};

}


#endif // SELECTCONTACT_H

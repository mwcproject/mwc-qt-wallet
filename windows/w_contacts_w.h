#ifndef CONTACTSW_H
#define CONTACTSW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace Ui {
class Contacts;
}

namespace state {
    class Contacts;
}

namespace wnd {

class Contacts : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Contacts(QWidget *parent, state::Contacts * state);
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
    state::Contacts * state;
    QVector<core::ContactRecord> contacts;
};

}

#endif // CONTACTSW_H

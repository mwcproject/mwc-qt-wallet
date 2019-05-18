#ifndef CONTACTSW_H
#define CONTACTSW_H

#include <QWidget>
#include "../wallet/wallet.h"

namespace Ui {
class Contacts;
}

namespace state {
    class Contacts;
}

namespace wnd {

class Contacts : public QWidget
{
    Q_OBJECT

public:
    explicit Contacts(QWidget *parent, state::Contacts * state);
    ~Contacts();

private slots:
    void on_addButton_clicked();
    void on_editButton_clicked();
    void on_removeButton_clicked();

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
    QVector<wallet::WalletContact> contacts;
};

}

#endif // CONTACTSW_H

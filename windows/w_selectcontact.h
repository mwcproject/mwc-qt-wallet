#ifndef SELECTCONTACT_H
#define SELECTCONTACT_H

#include <QDialog>
#include  "../wallet/wallet.h"

namespace Ui {
class SelectContact;
}

class QListWidgetItem;

namespace wnd {

class SelectContact : public QDialog
{
    Q_OBJECT

public:
    explicit SelectContact(QWidget *parent, const QVector<wallet::WalletContact> & contacts );
    ~SelectContact();

    wallet::WalletContact getSelectedContact() const {return selectedContact;}

private slots:
    void on_cancelButton_clicked();

    void on_selectButton_clicked();

    void on_contactsListWidget_itemActivated(QListWidgetItem *item);

private:
    Ui::SelectContact *ui;
    QVector<wallet::WalletContact> contacts;
    wallet::WalletContact selectedContact;
};

}


#endif // SELECTCONTACT_H

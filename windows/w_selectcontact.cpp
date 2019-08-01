#include "w_selectcontact.h"
#include "ui_w_selectcontact.h"
#include "../util/stringutils.h"
#include <QListWidgetItem>
#include "../control/messagebox.h"

namespace wnd {

SelectContact::SelectContact(QWidget *parent, const QVector<wallet::WalletContact> & _contacts ) :
    QDialog(parent),
    ui(new Ui::SelectContact),
    contacts(_contacts)
{
    ui->setupUi(this);

    int idx = 0;
    for ( const wallet::WalletContact & cnt : contacts ) {
        QListWidgetItem *aitem = new QListWidgetItem(
            util::expandStrR(cnt.name, 15) + cnt.address , ui->contactsListWidget );
        aitem->setData(Qt::UserRole, idx++);
        ui->contactsListWidget->addItem(aitem);
    }

}

SelectContact::~SelectContact()
{
    delete ui;
}

void SelectContact::on_cancelButton_clicked()
{
    reject();
}

void SelectContact::on_selectButton_clicked()
{
    QListWidgetItem * itm = ui->contactsListWidget->currentItem();
    if (itm==nullptr) {
        control::MessageBox::message(this, "Need info", "Please select a contact that you are going to use");
        return;
    }

    int idx = itm->data(Qt::UserRole).toInt();
    selectedContact = contacts[idx];
    accept();
}

void SelectContact::on_contactsListWidget_itemActivated(QListWidgetItem *item)
{
    int idx = item->data(Qt::UserRole).toInt();
    selectedContact = contacts[idx];
    accept();
}

}


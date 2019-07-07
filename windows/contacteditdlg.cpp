#include "windows/contacteditdlg.h"
#include "ui_contacteditdlg.h"
#include "../control/messagebox.h"

namespace wnd {

ContactEditDlg::ContactEditDlg(QWidget *parent, const wallet::WalletContact & _contact,
                               const QVector<wallet::WalletContact> & contacts, bool _editMode ) :
    QDialog(parent),
    ui(new Ui::ContactEditDlg),
    contact(_contact),
    contactList(contacts),
    editMode(_editMode)
{
    ui->setupUi(this);

    ui->nameEdit->setText(contact.name);
    ui->addressEdit->setText(contact.address);

    // In edit mode name we can't change
    ui->nameEdit->setReadOnly(editMode);
}

ContactEditDlg::~ContactEditDlg()
{
    delete ui;
}

void ContactEditDlg::on_applyButton_clicked()
{
    contact.name = ui->nameEdit->text().trimmed();
    contact.address = ui->addressEdit->text().trimmed();

    if ( contact.name.isEmpty() )
    {
        control::MessageBox::message(this, "Need Info", "Please specify a name for your contact");
        return;
    }

    if ( contact.address.isEmpty() )
    {
        control::MessageBox::message(this, "Need info", "Please specify an address for your contact");
        return;
    }

    if (!editMode) {
        for ( auto & cnt : contactList ) {
            if (cnt.name == contact.name) {
                control::MessageBox::message(this, "Names collision", "Contact with a name "+contact.name+
                                      " allready exist. Please specify unique name for your contact");
                return;
            }
        }
    }

    accept();
}

void ContactEditDlg::on_cancelButton_clicked()
{
    reject();
}

}


#include "w_contacteditdlg.h"
#include "ui_w_contacteditdlg.h"
#include "../control/messagebox.h"
#include "../util/address.h"

namespace wnd {

ContactEditDlg::ContactEditDlg(QWidget *parent, const core::ContactRecord & _contact,
                               const QVector<core::ContactRecord> & contacts, bool editMode ) :
    control::MwcDialog(parent),
    ui(new Ui::ContactEditDlg),
    contact(_contact),
    contactList(contacts)
{
    ui->setupUi(this);

    ui->nameEdit->setText(contact.name);
    ui->addressEdit->setText(contact.address);

    ui->titleLabel->setText( editMode ? "EDIT CONTACT" : "NEW CONTACT" );
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
        ui->nameEdit->setFocus();
        return;
    }

    if ( contact.address.isEmpty() )
    {
        control::MessageBox::message(this, "Need info", "Please specify an address for your contact");
        ui->addressEdit->setFocus();
        return;
    }

    QPair< bool, util::ADDRESS_TYPE > res = util::verifyAddress(contact.address);
    if ( !res.first ) {
        control::MessageBox::message(this, "Incorrect Input",
                                     "Please specify correct address for your contact" );
        ui->addressEdit->setFocus();
        return;
    }


    for ( auto & cnt : contactList ) {
            if (cnt.name == contact.name) {
                control::MessageBox::message(this, "Names collision", "Contact with a name "+contact.name+
                                      " allready exist. Please specify unique name for your contact");
                ui->nameEdit->setFocus();
                return;
            }
    }

    accept();
}

void ContactEditDlg::on_cancelButton_clicked()
{
    reject();
}

}


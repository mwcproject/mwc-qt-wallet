#include "contacts.h"
#include "ui_contacts.h"

namespace wnd {

Contacts::Contacts(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Contacts)
{
    ui->setupUi(this);
}

Contacts::~Contacts()
{
    delete ui;
}

}

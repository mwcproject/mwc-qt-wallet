#include "mwc_windows/contacts.h"
#include "ui_contacts.h"

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

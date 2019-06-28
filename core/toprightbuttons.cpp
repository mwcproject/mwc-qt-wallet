#include "toprightbuttons.h"
#include "ui_toprightbuttons.h"

namespace core {

TopRightButtons::TopRightButtons(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TopRightButtons),
        prntWnd(parent)
{
    ui->setupUi(this);
}

TopRightButtons::~TopRightButtons() {
    delete ui;
}

void TopRightButtons::checkButton(BTN b) {
    ui->accountButton->setChecked(b==BTN::ACCOUNTS);
    ui->notificationButton->setChecked(b==BTN::NOTIFICATION);
    ui->settingsButton->setChecked(b==BTN::SETTINGS);
}

void TopRightButtons::on_notificationButton_clicked()
{
    checkButton(BTN::NOTIFICATION);
}

void TopRightButtons::on_settingsButton_clicked()
{
    checkButton(BTN::SETTINGS);
}

void TopRightButtons::on_accountButton_clicked()
{
    checkButton(BTN::ACCOUNTS);
}


}

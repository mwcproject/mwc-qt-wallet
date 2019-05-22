#include "inputpassword_w.h"
#include "ui_inputpassword.h"
#include "../state/inputpassword.h"
#include <QMessageBox>
#include <QThread>
#include <QShortcut>
#include "../util/widgetutils.h"

namespace wnd {

InputPassword::InputPassword(QWidget *parent, state::InputPassword * _state) :
    QWidget(parent),
    ui(new Ui::InputPassword),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Login");

    setFocusPolicy(Qt::StrongFocus);
    ui->passwordEdit->setFocus(Qt::OtherFocusReason);

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
}

InputPassword::~InputPassword()
{
    delete ui;
}

void InputPassword::on_submitButton_clicked()
{
    QString pswd = ui->passwordEdit->text();

    if (! state->checkPassword(pswd) ) {
        QMessageBox::critical(this, "Password", "Password doesn't match our records. Please input correct password.");


        QThread::sleep(1); // sleep to prevent brute force attack.
                        // Note, we are using small hash, so the brute force attach will likely
                        // found wong password with similar hash.

        ui->passwordEdit->setText("");
        ui->passwordEdit->setFocus(Qt::OtherFocusReason);
        return;

    }

    state->submitPassword(pswd);

}


}


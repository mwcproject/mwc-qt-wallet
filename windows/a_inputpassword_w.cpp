#include "a_inputpassword_w.h"
#include "ui_a_inputpassword.h"
#include "../state/a_inputpassword.h"
#include "../control/messagebox.h"
#include <QThread>
#include <QShortcut>
#include "../util/widgetutils.h"
#include <QMovie>
#include <QLabel>
#include "../core/global.h"

namespace wnd {

InputPassword::InputPassword(QWidget *parent, state::InputPassword * _state) :
    QWidget(parent),
    ui(new Ui::InputPassword),
    state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    state->setWindowTitle( mwc::VERSION + " " + mwc::APP_NAME );

    setFocusPolicy(Qt::StrongFocus);
    ui->passwordEdit->setFocus(Qt::OtherFocusReason);

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
}

InputPassword::~InputPassword()
{
    state->deleteWnd();

    delete ui;
}

void InputPassword::on_submitButton_clicked() {
    QString pswd = ui->passwordEdit->text();

    // Submit the password and wait until state will push us.
    state->submitPassword(pswd);
    // Because of event driven, the flow is not linear
}

void InputPassword::startWaiting() {
    ui->progress->show();
}

void InputPassword::stopWaiting() {
    ui->progress->hide();
}

void InputPassword::reportWrongPassword() {
    control::MessageBox::message(this, "Password", "Password supplied was incorrect. Please input correct password.");

    QThread::sleep(1); // sleep to prevent brute force attack.
    // Note, we are using small hash, so the brute force attach will likely
    // found wong password with similar hash.

    ui->passwordEdit->setText("");
    ui->passwordEdit->setFocus(Qt::OtherFocusReason);
}

void InputPassword::on_instancesButton_clicked()
{
    control::MessageBox::message(this, "Not implemented", "Here we will have a dialog or a page where it will be possible to specify the directory with wallet data. Somilar to electrum");
}


}



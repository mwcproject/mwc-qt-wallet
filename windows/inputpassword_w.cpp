#include "inputpassword_w.h"
#include "ui_inputpassword.h"
#include "../state/inputpassword.h"
#include <QMessageBox>
#include <QThread>
#include <QShortcut>
#include "../util/widgetutils.h"
#include <QMovie>
#include <QLabel>

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

    QLabel *lbl = ui->progress;
    QMovie *movie = new QMovie(":/img/loader.gif", QByteArray(), this);
    lbl->setMovie(movie);
    movie->start();
    lbl->hide();

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
}

InputPassword::~InputPassword()
{
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
    QMessageBox::critical(this, "Password", "Password doesn't match our records. Please input correct password.");


    QThread::sleep(1); // sleep to prevent brute force attack.
    // Note, we are using small hash, so the brute force attach will likely
    // found wong password with similar hash.

    ui->passwordEdit->setText("");
    ui->passwordEdit->setFocus(Qt::OtherFocusReason);
}


}


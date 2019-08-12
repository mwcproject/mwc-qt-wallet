#include "hodl_w.h"
#include "ui_hodl.h"
#include "../state/hodl.h"
#include "../control/messagebox.h"
#include <QAbstractItemView>
#include "../state/timeoutlock.h"
#include <QProcess>

namespace wnd {

Hodl::Hodl(QWidget *parent, state::Hodl * _state) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::Hodl),
    state(_state)
{
    ui->setupUi(this);

}

Hodl::~Hodl()
{
    delete ui;
}

void Hodl::on_pushButton_clicked()
{
    QProcess::startDetached("open https://www.mwc.mw/contact");
}


}



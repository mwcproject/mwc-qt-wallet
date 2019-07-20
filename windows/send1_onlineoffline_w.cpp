#include "send1_onlineoffline_w.h"
#include "ui_send1_onlineoffline.h"
#include "../state/send1_OnlineOffline.h"

namespace wnd {

SendOnlineOffline::SendOnlineOffline(QWidget *parent, state::SendOnlineOffline * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::SendOnlineOffline),
    state(_state)
{
    ui->setupUi(this);
    ui->progress->initLoader(false);
}

SendOnlineOffline::~SendOnlineOffline()
{
    state->destroyWnd(this);
    delete ui;
}

void SendOnlineOffline::showProgress() {
    ui->progress->show();
}


void SendOnlineOffline::on_sendOnlineButton_clicked()
{
    state->SendOnlineChosen();
}

void SendOnlineOffline::on_sendOfflineButton_clicked()
{
    state->SendOfflineChosen();
}

}

#include "events_w.h"
#include "ui_events.h"
#include "../state/events.h"

namespace wnd {

Events::Events(QWidget *parent, state::Events * _state) :
    QWidget(parent),
    ui(new Ui::Events),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Events");

    ui->notifications->setFocus();
}

Events::~Events()
{
    delete ui;
}

}


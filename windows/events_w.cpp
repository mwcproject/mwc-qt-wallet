#include "events_w.h"
#include "ui_events.h"

namespace wnd {

Events::Events(QWidget *parent, state::Events * _state) :
    QWidget(parent),
    ui(new Ui::Events),
    state(_state)
{
    ui->setupUi(this);
}

Events::~Events()
{
    delete ui;
}

}


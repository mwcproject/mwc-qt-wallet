#ifndef EVENTSW_H
#define EVENTSW_H

#include <QWidget>

namespace Ui {
class Events;
}

namespace state {
    class Events;
}

namespace wnd {

class Events : public QWidget
{
    Q_OBJECT

public:
    explicit Events(QWidget *parent, state::Events * state);
    ~Events();

private:
    Ui::Events *ui;
    state::Events * state;
};

}

#endif // EVENTS_H

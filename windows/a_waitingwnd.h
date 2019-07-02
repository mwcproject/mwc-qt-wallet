#ifndef WAITINGWND_H
#define WAITINGWND_H

#include <QWidget>

namespace Ui {
class WaitingWnd;
}

namespace wnd {

class WaitingWndState {
public:
    void virtual deleteWaitingWnd() = 0;
};

// Just a waiting dialog, that can be updated with some info
class WaitingWnd : public QWidget
{
    Q_OBJECT

public:
    explicit WaitingWnd(QWidget *parent, WaitingWndState * state, QString title, QString progressMessage);
    ~WaitingWnd();

    void updateTitle(QString title);
    void updateProgressMessage( QString message );

private:
    Ui::WaitingWnd *ui;
    WaitingWndState * state;
};

}

#endif // WAITINGWND_H

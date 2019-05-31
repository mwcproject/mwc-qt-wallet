#ifndef WAITINGWND_H
#define WAITINGWND_H

#include <QWidget>

namespace Ui {
class WaitingWnd;
}

namespace wnd {

// Just a waiting dialog, that can be updated with some info
class WaitingWnd : public QWidget
{
    Q_OBJECT

public:
    explicit WaitingWnd(QWidget *parent, QString title, QString progressMessage);
    ~WaitingWnd();

    void updateTitle(QString title);
    void updateProgressMessage( QString message );

private:
    Ui::WaitingWnd *ui;
};

}

#endif // WAITINGWND_H

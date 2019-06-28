#ifndef NAVWND_H
#define NAVWND_H

#include <QWidget>

namespace core {

class TopRightButtons;

// Window with navigation bar
class NavWnd : public QWidget {
Q_OBJECT
public:
    explicit NavWnd(QWidget *parent);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    TopRightButtons * topRightButtonWnd = nullptr;
};

}

#endif // NAVWND_H

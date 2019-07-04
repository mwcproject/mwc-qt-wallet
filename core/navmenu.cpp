#include "navmenu.h"
#include <QMouseEvent>
#include <QApplication>

namespace core {

NavMenu::NavMenu(QWidget *parent) : QFrame(parent)
{
    setWindowFlags( Qt::WindowStaysOnTopHint);
    setFocus();
}

void NavMenu::focusOutEvent(QFocusEvent *event) {
    Q_UNUSED(event);
    emit closed();
    close();

}

void NavMenu::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    emit closed();
    close();
}



}

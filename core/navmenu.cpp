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
    emit closed();
    close();

}

void NavMenu::leaveEvent(QEvent *event) {
    emit closed();
    close();
}



}

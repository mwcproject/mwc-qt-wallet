#include "navwnd.h"
#include "navbar.h"
#include <QResizeEvent>
#include "navmenuaccount.h"

namespace core {

// Offset in pixels
const int TOP_RIGHT_BTNS_CX = 40;
const int TOP_RIGHT_BTNS_CY = 15;

const int NAV_MENU_WIDTH = 190;

NavWnd::NavWnd(QWidget *parent, state::StateMachine * stateMachine, bool createNavigationButtons) : QWidget(parent)
{
    if (createNavigationButtons) {
        topRightButtonWnd = new NavBar(this, stateMachine);
    }
}

void NavWnd::resizeEvent(QResizeEvent *event) {

    if (topRightButtonWnd!= nullptr) {
        QSize sz = event->size();

        QSize trwndSz = topRightButtonWnd->size();
        int x0 = sz.width() - trwndSz.width() - TOP_RIGHT_BTNS_CX;
        topRightButtonWnd->move(x0, TOP_RIGHT_BTNS_CY);

        topRightButtonWnd->setNavMenuPosition(QPoint(std::min(x0, sz.width() - NAV_MENU_WIDTH - TOP_RIGHT_BTNS_CY),
                                                     TOP_RIGHT_BTNS_CY + trwndSz.height() + trwndSz.height() / 3));
    }
}


}



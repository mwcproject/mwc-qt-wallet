#include "navwnd.h"
#include "toprightbuttons.h"
#include <QResizeEvent>

namespace core {

// Offset in pixels
const int TOP_RIGHT_BTNS_CX = 40;
const int TOP_RIGHT_BTNS_CY = 15;

NavWnd::NavWnd(QWidget *parent) : QWidget(parent)
{
    topRightButtonWnd = new TopRightButtons(this);
}

void NavWnd::resizeEvent(QResizeEvent *event) {

    QSize sz = event->size();

    QSize trwndSz = topRightButtonWnd->size();
    topRightButtonWnd->move( sz.width() - trwndSz.width() - TOP_RIGHT_BTNS_CX, TOP_RIGHT_BTNS_CY );
}


}



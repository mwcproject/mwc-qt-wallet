// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "navwnd.h"
#include "navbar.h"
#include <QResizeEvent>
#include "navmenuaccount.h"

namespace core {

// Offset in pixels
const int TOP_RIGHT_BTNS_CX = 40;
const int TOP_RIGHT_BTNS_CY = 15;

const int NAV_MENU_WIDTH = 190;

NavWnd::NavWnd(QWidget *parent,  state::StateContext * context, bool createNavigationButtons, bool showFinalizeButton) : QWidget(parent)
{
    if (createNavigationButtons) {
        topRightButtonWnd = new NavBar(this, context, showFinalizeButton);
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



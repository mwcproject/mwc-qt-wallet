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

// Copyright 2020 The MWC Developers
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

#include "PanelBaseWnd.h"
#include <QTimer>

namespace core {

PanelBaseWnd::PanelBaseWnd( QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f)
{
    updateAppTitle();
    QTimer::singleShot(0, this, SLOT(updateAppTitle()));
}

void PanelBaseWnd::updateAppTitle() {
#ifdef Q_OS_DARWIN
    Cocoa::changeTitleBarColor(winId(), 0x6F/255.0, 0.0, 0xD6/255.0 ); // Color #6f00d6
#endif
    setFocusPolicy(Qt::StrongFocus);
    this->setFocus();
    panelWndStarted();
}

}

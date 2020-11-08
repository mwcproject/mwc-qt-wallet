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

#ifndef NAVWND_H
#define NAVWND_H

#include "PanelBaseWnd.h"

namespace core {

class NavBar;
class NavMenuAccount;
class AppContext;
class PanelTitle;

// Window with navigation bar
class NavWnd : public PanelBaseWnd {
Q_OBJECT
public:
    explicit NavWnd(QWidget *parent, bool createNavigationButtons=true);

protected:
    void setPageTitle(QString title);

    virtual void resizeEvent(QResizeEvent *event) override;

private:
    NavBar * topRightButtonWnd = nullptr;
    PanelTitle * panelTitle = nullptr;
};

}

#endif // NAVWND_H

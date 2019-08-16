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

#ifndef TOPRIGHTBUTTONS_H
#define TOPRIGHTBUTTONS_H

#include <QWidget>

namespace Ui {
class NavBar;
}

namespace state {
struct StateContext;
}

namespace core {

class AppContext;
class NavMenu;

class NavBar : public QWidget {
Q_OBJECT

public:
    explicit NavBar(QWidget *parent, state::StateContext * context );

    ~NavBar();

    void setNavMenuPosition( const QPoint & pos ) { navMenuPos = pos; }

private slots:
    void on_notificationButton_clicked();
    void on_settingsButton_clicked();
    void on_accountButton_clicked();

    void onUpdateNonShownWarnings(bool hasNonShownWarns);

    void onMenuDestroy();
private:
    enum BTN {NONE, NOTIFICATION, SETTINGS, ACCOUNTS};
    void checkButton(BTN b);

    void showNavMenu( NavMenu * menu );
private:
    Ui::NavBar *ui;
    QWidget *prntWnd; // will be used for popup
    state::StateContext * context = nullptr;
    QPoint navMenuPos; // Position of navigate menu
};

}

#endif // TOPRIGHTBUTTONS_H

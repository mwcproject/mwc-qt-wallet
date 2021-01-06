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

#include <state/state.h>
#include "navbar.h"
#include "ui_navbar.h"
#include "navmenuconfig.h"
#include "navmenuaccount.h"
#include "../util/Json.h"
#include "../bridge/config_b.h"
#include "../bridge/statemachine_b.h"
#include "../bridge/wnd/x_events_b.h"

using namespace bridge;

namespace core {

NavBar::NavBar(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::NavBar),
        prntWnd(parent)
{
    config = new Config(this);
    stateMachine = new StateMachine(this);
    events = new Events(this);

    if (config->isOnlineNode()) {
        ui->setupUi(this);

        ui->accountButton->hide();
    }
    else {
        QObject::connect( events, &Events::sgnUpdateNonShownWarnings,
                          this, &NavBar::onUpdateNonShownWarnings, Qt::QueuedConnection );

        ui->setupUi(this);

        onUpdateNonShownWarnings( events->hasNonShownWarnings() );
    }
}

NavBar::~NavBar() {
    delete ui;
}

void NavBar::checkButton(BTN b) {
    ui->accountButton->setChecked(b==BTN::ACCOUNTS);
    ui->notificationButton->setChecked(b==BTN::NOTIFICATION);
    ui->settingsButton->setChecked(b==BTN::SETTINGS);
}

void NavBar::onUpdateNonShownWarnings(bool hasNonShownWarns) {
    ui->notificationButton->setIcon( QIcon( QPixmap(
            hasNonShownWarns ? ":/img/NavNotificationActive@2x.svg" : ":/img/NavNotificationNormal@2x.svg" ) ) );
}

void NavBar::on_notificationButton_clicked()
{
    checkButton(BTN::NOTIFICATION);
    stateMachine->setActionWindow( state::STATE::EVENTS );
}

void NavBar::on_settingsButton_clicked()
{
    checkButton(BTN::SETTINGS);

    showNavMenu( new NavMenuConfig( prntWnd ) );
}

void NavBar::on_accountButton_clicked()
{
    checkButton(BTN::ACCOUNTS);

    showNavMenu( new NavMenuAccount( prntWnd ) );
}

void NavBar::onMenuDestroy() {
    checkButton(BTN::NONE);
}


void NavBar::showNavMenu( NavMenu * menu ) {
    menu->move( navMenuPos );
    QObject::connect( menu, SIGNAL(closed()), this, SLOT(onMenuDestroy()));
    menu->show();
}


}


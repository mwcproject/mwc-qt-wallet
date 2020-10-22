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

#include "x_events_w.h"
#include "ui_x_events.h"
#include "../dialogs_desktop/x_shownotificationdlg.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/x_events_b.h"
#include "../control_desktop/richvbox.h"
#include "../control_desktop/richitem.h"
#include <QLabel>

const int MSG_GROUP_SIZE = 5;

namespace wnd {

Events::Events(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Events)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    events = new bridge::Events(this);

    ui->eventsList->setFocus();

    QObject::connect(ui->eventsList, &control::RichVBox::onItemActivated,
                     this, &Events::onItemActivated, Qt::QueuedConnection);

    updateShowMessages();
}

Events::~Events()
{
    events->eventsWndIsDeleted();
    delete ui;
}

void Events::updateShowMessages() {

    messages = events->getWalletNotificationMessages();

  //  ui->eventsList->clearAll();

    // Current implementaiton of the list with bunch of controls can't have too many items.
    // That is why we printing only first 200 elements.
    int msgNumber = 0;
    for (int i=messages.size()/MSG_GROUP_SIZE-1; i>=0 && msgNumber < 5000; i--, msgNumber++ ) {
        QString timeShort = messages[i*MSG_GROUP_SIZE];
        QString timeLong = messages[i*MSG_GROUP_SIZE+1];
        QString levelShort = messages[i*MSG_GROUP_SIZE+2];
        QString levelFull = messages[i*MSG_GROUP_SIZE+3];
        QString message = messages[i*MSG_GROUP_SIZE+4];

#ifndef QT_DEBUG
        if ( levelShort.contains("dbg") )
            continue; // Don't want to show debug messaged in the release.
#endif

        control::RichItem * itm = control::createMarkedItem(QString::number(i*MSG_GROUP_SIZE), ui->eventsList, !(levelShort == "info" || levelShort == "dgb") );
        itm->addWidget( control::createLabel(itm, true, true, timeLong + " / " + levelFull ) )
            .addWidget( control::createLabel(itm, true, false, message ) );
        itm->apply();

        ui->eventsList->addItem(itm);
    }
    ui->eventsList->apply();
}

void Events::onItemActivated(QString itemId) {
    util::TimeoutLockObject to("Events");

    int idx = itemId.toInt();

    // Show message details for that row
    if (idx>=0 && idx<=messages.size() - MSG_GROUP_SIZE) {
        dlg::ShowNotificationDlg * showDlg = new dlg::ShowNotificationDlg( messages[idx+1],
                                                                           messages[idx+3], messages[idx+4], this );
        showDlg->exec();
        delete(showDlg);
    }

}

}



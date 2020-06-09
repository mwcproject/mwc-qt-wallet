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

const int MSG_GROUP_SIZE = 5;

namespace wnd {

Events::Events(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Events)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    events = new bridge::Events(this);

    initTableHeaders();

    ui->notificationList->setTextAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    ui->notificationList->setFocus();

    updateShowMessages();
}

Events::~Events()
{
    saveTableHeaders();

    events->eventsWndIsDeleted();
    delete ui;
}

void Events::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = config->getColumnsWidhts("NotifTblColWidth");
    if ( widths.size() != 3 ) {
        widths = QVector<int>{80, 50, 550 };
    }
    Q_ASSERT( widths.size() == 3 );

    ui->notificationList->setColumnWidths( widths );
}

void Events::saveTableHeaders() {
    config->updateColumnsWidhts("NotifTblColWidth", ui->notificationList->getColumnWidths());
}

void Events::updateShowMessages() {

    messages = events->getWalletNotificationMessages();

    ui->notificationList->clearData();

    for (int i=messages.size()/MSG_GROUP_SIZE-1; i>=0; i-- ) {
        QString timeShort = messages[i*MSG_GROUP_SIZE];
        QString levelShort = messages[i*MSG_GROUP_SIZE+2];
        QString message = messages[i*MSG_GROUP_SIZE+4];

#ifndef QT_DEBUG
        if ( levelShort.contains("dbg") )
            continue; // Don't want ot show debug messaged in the release.
#endif
        ui->notificationList->appendRow( QVector<QString>{ timeShort, levelShort, message }, 0.0);
    }
}

void Events::on_notificationList_cellActivated(int row, int column)
{
    util::TimeoutLockObject to("Events");

    Q_UNUSED(column);
    // Show message details for that row
    if (row>=0 && row<messages.size()/MSG_GROUP_SIZE) {
        int groupIdx = (messages.size()/MSG_GROUP_SIZE-1-row) * MSG_GROUP_SIZE;
        dlg::ShowNotificationDlg * showDlg = new dlg::ShowNotificationDlg( messages[groupIdx+1],
                messages[groupIdx+3], messages[groupIdx+4], this );
        showDlg->exec();
        delete(showDlg);
    }
}

}



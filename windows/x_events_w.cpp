#include "x_events_w.h"
#include "ui_x_events.h"
#include "../state/x_events.h"
#include "../wallet/wallet.h"
#include "../dialogs/shownotificationdlg.h"

namespace wnd {

Events::Events(QWidget *parent, state::Events * _state) :
    core::NavWnd( parent, _state->getContext() ),
    ui(new Ui::Events),
    state(_state)
{
    ui->setupUi(this);

    initTableHeaders();

    ui->notificationList->setTextAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    ui->notificationList->setFocus();

    updateShowMessages();
}

Events::~Events()
{
    saveTableHeaders();
    state->deleteEventsWnd(this);
    delete ui;
}

void Events::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 3 ) {
        widths = QVector<int>{80, 50, 550 };
    }
    Q_ASSERT( widths.size() == 3 );

    ui->notificationList->setColumnWidths( widths );
}

void Events::saveTableHeaders() {
    state->updateColumnsWidhts(ui->notificationList->getColumnWidths());
}

void Events::updateShowMessages() {

    messages = state->getWalletNotificationMessages();
    int64_t watermarkTime = state->getWatermarkTime();

    ui->notificationList->clearData();

    for (int i=messages.size()-1; i>=0; i-- ) {
        wallet::WalletNotificationMessages msg = messages[i];

#ifndef QT_DEBUG
        if ( msg.level >= wallet::WalletNotificationMessages::DEBUG )
            continue; // Don't want ot show debug messaged in the release.
#endif

        bool newMsg = msg.time.toMSecsSinceEpoch() > watermarkTime;
        bool sevMsg = msg.isCritical();
        // 0 -nothing, 1 - hightlight
        double selection = 0.0 + (newMsg?0.15:0.0) + (sevMsg?0.3:0.0);

        // time; level; message
        ui->notificationList->appendRow( QVector<QString>{ msg.time.toString("HH:mm:ss"), msg.getLevelStr(), msg.message }, selection);
    }
}

void Events::on_notificationList_cellActivated(int row, int column)
{
    Q_UNUSED(column);
    // Show message details for that row
    if (row>=0 && row<messages.size()) {

        dlg::ShowNotificationDlg * showDlg = new dlg::ShowNotificationDlg( messages[row], this );
        showDlg->exec();
        delete(showDlg);
    }
}

}



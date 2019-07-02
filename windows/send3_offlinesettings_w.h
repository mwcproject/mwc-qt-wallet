#ifndef SEND3_OFFLINESETTINGS_H
#define SEND3_OFFLINESETTINGS_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class SendOfflineSettings;
}

namespace state {
class SendOffline;
}


namespace wnd {

class SendOfflineSettings : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendOfflineSettings(QWidget *parent, state::SendOffline * state);
    virtual ~SendOfflineSettings() override;

private slots:
    void on_settingsButton_clicked();
    void on_sendButton_clicked();

private:
    Ui::SendOfflineSettings * ui;
    state::SendOffline * state;
    QVector<wallet::AccountInfo> accountInfo;
};

}

#endif // SEND3_OFFLINESETTINGS_H

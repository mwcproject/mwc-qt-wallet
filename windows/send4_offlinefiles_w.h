#ifndef SEND4_OFFLINEFILES_H
#define SEND4_OFFLINEFILES_H

#include <QWidget>
#include "../wallet/wallet.h"

namespace Ui {
class SendOfflineFiles;
}

namespace state {
class SendOffline;
}

namespace wnd {

class SendOfflineFiles : public QWidget
{
    Q_OBJECT

public:
    explicit SendOfflineFiles(QWidget *parent, const wallet::AccountInfo & account, state::SendOffline * state);
    virtual ~SendOfflineFiles() override;

    void onTransactionActionIsFinished( bool success, QString message );
private slots:
    void on_generateFileButton_clicked();
    //void on_signTransactionButton_clicked();
    void on_publishTransactionButton_clicked();

private:
    Ui::SendOfflineFiles *ui;
    state::SendOffline * state;
    wallet::AccountInfo account;
};

}

#endif // SEND4_OFFLINEFILES_H

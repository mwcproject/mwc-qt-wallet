#ifndef CONTACTEDITDLG_H
#define CONTACTEDITDLG_H

#include <QDialog>
#include "../wallet/wallet.h"

namespace Ui {
class ContactEditDlg;
}

namespace wnd {

class ContactEditDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ContactEditDlg(QWidget *parent, const wallet::WalletContact & contact,
                            const QVector<wallet::WalletContact> & contacts, bool editMode );
    ~ContactEditDlg();

    const wallet::WalletContact & getContact() const {return contact;}

private slots:
    void on_applyButton_clicked();
    void on_cancelButton_clicked();
private:
    Ui::ContactEditDlg *ui;
    wallet::WalletContact contact;
    const QVector<wallet::WalletContact> contactList;
    const bool editMode;
};

}

#endif // CONTACTEDITDLG_H

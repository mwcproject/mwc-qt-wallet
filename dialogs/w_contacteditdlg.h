#ifndef CONTACTEDITDLG_H
#define CONTACTEDITDLG_H

#include <QDialog>
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../control/mwcdialog.h"

namespace Ui {
class ContactEditDlg;
}

namespace wnd {

class ContactEditDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit ContactEditDlg(QWidget *parent, const core::ContactRecord & contact,
                            const QVector<core::ContactRecord> & contacts, bool editMode );
    virtual ~ContactEditDlg() override;

    const core::ContactRecord & getContact() const {return contact;}

private slots:
    void on_applyButton_clicked();
    void on_cancelButton_clicked();
private:
    Ui::ContactEditDlg *ui;
    core::ContactRecord contact;
    const QVector<core::ContactRecord> contactList;
};

}

#endif // CONTACTEDITDLG_H

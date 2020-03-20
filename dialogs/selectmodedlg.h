#ifndef SELECTMODEDLG_H
#define SELECTMODEDLG_H

#include "../core/Config.h"
#include "../control/mwcdialog.h"

namespace Ui {
class SelectModeDlg;
}

namespace dlg {

class SelectModeDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit SelectModeDlg(QWidget *parent = nullptr);
    ~SelectModeDlg();

    config::WALLET_RUN_MODE getRunMod() const {return runMode;}
private slots:
    void on_cancelButton_clicked();

    void on_submitButton_clicked();

private:
    Ui::SelectModeDlg *ui;
    config::WALLET_RUN_MODE runMode = config::WALLET_RUN_MODE::ONLINE_WALLET;
};

}

#endif // SELECTMODEDLG_H

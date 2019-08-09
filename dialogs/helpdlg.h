#ifndef HELPDLG_H
#define HELPDLG_H

#include <QDialog>
#include "../control/mwcdialog.h"

namespace Ui {
class HelpDlg;
}

namespace dlg {

class HelpDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    // documentName - doc name to display:  XXXXXX.html. Expected to be found at resources qrc:/help/XXXXXX.html
    explicit HelpDlg( QWidget *parent, QString documentName );
    ~HelpDlg();

private slots:
    void on_backwardBtn_clicked();
    void on_forwardBtn_clicked();
    void on_doneButton_clicked();

    void on_backwardAvailable(bool available);
    void on_forwardAvailable(bool available);

private:
    Ui::HelpDlg *ui;
};

}

#endif // HELPDLG_H

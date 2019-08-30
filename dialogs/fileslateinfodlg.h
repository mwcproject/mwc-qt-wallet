#ifndef FILESLATEINFODLG_H
#define FILESLATEINFODLG_H

#include "control/mwcdialog.h"

namespace Ui {
class FileSlateInfoDlg;
}

namespace util {
struct FileTransactionInfo;
}

namespace dlg {

// !!!!! Depricated, using page now
////////////////////////////////////////////
class FileSlateInfoDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit FileSlateInfoDlg(QWidget *parent, const QString & title, const util::FileTransactionInfo & flTrInfo);
    ~FileSlateInfoDlg();

private slots:
    void on_cancelBtn_clicked();

    void on_processBtn_clicked();

private:
    Ui::FileSlateInfoDlg *ui;
};

}


#endif // FILESLATEINFODLG_H

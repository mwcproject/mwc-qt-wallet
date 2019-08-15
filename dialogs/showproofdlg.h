#ifndef SHOWPROOFDLG_H
#define SHOWPROOFDLG_H

#include "../control/mwcdialog.h"

namespace Ui {
class ShowProofDlg;
}

namespace dlg {

struct ProofInfo {
    QString mwc;
    QString toAdress;
    QString fromAdress;

    QString output;
    QString kernel;

    // Parse the data and fill the structure.
    bool parseProofText(const QString & proof);
};

class ShowProofDlg : public control::MwcDialog {
Q_OBJECT

public:
    explicit ShowProofDlg(QWidget *parent, const QString &fileName, const ProofInfo & proofInfo);

    ~ShowProofDlg();

private slots:
    void on_viewOutput_clicked();

    void on_viewKernel_clicked();

    void on_pushButton_clicked();

private:
    Ui::ShowProofDlg *ui;
    ProofInfo proof;
};

}

#endif // SHOWPROOFDLG_H

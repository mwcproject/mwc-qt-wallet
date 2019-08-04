#ifndef X_WALLETINSTANCES_H
#define X_WALLETINSTANCES_H

#include "../control/mwcdialog.h"

namespace Ui {
class WalletInstances;
}

namespace state {
class WalletConfig;
}

namespace dlg {

class WalletInstances : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit WalletInstances(QWidget *parent, state::WalletConfig * state);
    ~WalletInstances();

private slots:
    void on_mwc713directorySelect_clicked();

    void on_cancelButton_clicked();

    void on_applyButton_clicked();

private:
    Ui::WalletInstances *ui;
    state::WalletConfig * state;
};

}


#endif // X_WALLETINSTANCES_H

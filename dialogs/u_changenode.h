#ifndef U_CHANGENODE_H
#define U_CHANGENODE_H

#include "../control/mwcdialog.h"
#include "../wallet/wallet.h"

namespace Ui {
class ChangeNode;
}

namespace dlg {

class ChangeNode : public control::MwcDialog {
    Q_OBJECT
public:
    explicit ChangeNode(QWidget *parent, const wallet::WalletConfig & config);
    ~ChangeNode();

    wallet::WalletConfig getConfig() const {return config;}

private slots:

    void on_resetButton_clicked();
    void on_cancelButton_clicked();
    void on_applyButton_clicked();

private:
    Ui::ChangeNode *ui;

    wallet::WalletConfig config;
};

}

#endif // U_CHANGENODE_H

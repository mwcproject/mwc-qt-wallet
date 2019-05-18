#ifndef WALLETCONFIGW_H
#define WALLETCONFIGW_H

#include <QWidget>

namespace Ui {
class WalletConfig;
}

namespace state {
    class WalletConfig;
}

namespace wnd {

class WalletConfig : public QWidget
{
    Q_OBJECT

public:
    explicit WalletConfig(QWidget *parent, state::WalletConfig * state);
    ~WalletConfig();

private slots:
    void on_updateButton_clicked();

private:
    Ui::WalletConfig *ui;
    state::WalletConfig * state;
};

}

#endif // WALLETCONFIGW_H

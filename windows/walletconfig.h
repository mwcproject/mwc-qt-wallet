#ifndef WALLETCONFIG_H
#define WALLETCONFIG_H

#include <QWidget>

namespace Ui {
class WalletConfig;
}

namespace wnd {

class WalletConfig : public QWidget
{
    Q_OBJECT

public:
    explicit WalletConfig(QWidget *parent = nullptr);
    ~WalletConfig();

private:
    Ui::WalletConfig *ui;
};

}

#endif // WALLETCONFIG_H

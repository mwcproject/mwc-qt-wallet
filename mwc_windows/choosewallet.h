#ifndef CHOOSEWALLET_H
#define CHOOSEWALLET_H

#include <QWidget>
#include "../wnd_core/walletwindow.h"

namespace Ui {
class ChooseWallet;
}

class WalletInfo;

class ChooseWallet : public WalletWindow
{
    Q_OBJECT

public:
    explicit ChooseWallet( WalletWindowType appType, QWidget *parent );
    virtual ~ChooseWallet() override;

    const QString & getWalletPath() const {return walletPath;}

protected:
    virtual bool validateData() override;

private slots:
    void on_pushButton_clicked();

private:
    Ui::ChooseWallet *ui;
    QString walletPath;
};

#endif // CHOOSEWALLET_H

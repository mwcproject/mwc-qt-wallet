#ifndef NEWWALLET_H
#define NEWWALLET_H

#include "../wnd_core/walletwindow.h"

namespace Ui {
class NewWallet;
}


class NewWallet : public WalletWindow
{
    Q_OBJECT

public:
    enum NewWalletChoice {NEW_SEED, HAVE_SEED, SEED_FILE };


    explicit NewWallet(WalletWindowType appType, QWidget *parent);
    virtual ~NewWallet() override;

    NewWalletChoice getChoice() const {return choice;}

    virtual bool validateData() override;

private:
    Ui::NewWallet *ui;
    NewWalletChoice choice = NEW_SEED;
};

#endif // NEWWALLET_H

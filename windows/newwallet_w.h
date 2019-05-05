#ifndef NEWWALLETW_H
#define NEWWALLETW_H

#include <QWidget>

namespace Ui {
class NewWallet;
}

namespace state {
    class NewWallet;
}

namespace wnd {

class NewWallet : public QWidget
{
    Q_OBJECT

public:
    enum NewWalletChoice {NEW_SEED, HAVE_SEED, SEED_FILE };

    explicit NewWallet(QWidget *parent, state::NewWallet * state);
    virtual ~NewWallet() override;

private slots:
    void on_submitButton_clicked();

    void on_radioHaveSeed_clicked();
    void on_radioCreateNew_clicked();

private:
    void updateControls();
private:
    Ui::NewWallet *ui;
    state::NewWallet * state = nullptr;
};

}

#endif // NEWWALLET_H

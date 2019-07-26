#ifndef NEWWALLETW_H
#define NEWWALLETW_H

#include <QWidget>

namespace Ui {
class NewWallet;
}

namespace state {
    class InitAccount;
}

namespace wnd {

class NewWallet : public QWidget
{
    Q_OBJECT

public:
    explicit NewWallet(QWidget *parent, state::InitAccount * state);
    virtual ~NewWallet() override;

private slots:
    void on_submitButton_clicked();

    void on_radioHaveSeed_clicked();
    void on_radioCreateNew_clicked();

private:
    void updateControls();
private:
    Ui::NewWallet *ui;
    state::InitAccount * state = nullptr;
};

}

#endif // NEWWALLET_H

#ifndef HODLW_H
#define HODLW_H

#include <QWidget>
#include "../wallet/wallet.h"

namespace Ui {
class Hodl;
}

namespace state {
    class Hodl;
}

namespace wnd {

class Hodl : public QWidget
{
    Q_OBJECT

public:
    explicit Hodl(QWidget *parent, state::Hodl * state );
    ~Hodl();

private slots:

    void on_submitAllTransactionsButton_clicked();

    void on_submitSelectedTransactionsButton_clicked();

private:
    Ui::Hodl *ui;
    state::Hodl * state;
    QVector<wallet::WalletTransaction> transactions;
};

}

#endif // HODL_H

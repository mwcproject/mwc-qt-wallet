#ifndef MWCTOOLBAR_H
#define MWCTOOLBAR_H

#include <QWidget>
#include "../state/state.h"

namespace Ui {
class MwcToolbar;
}

namespace state {
class StateMachine;
}

namespace core {

class MainWindow;

class MwcToolbar : public QWidget
{
   Q_OBJECT

public:
    explicit MwcToolbar(QWidget *parent);
    ~MwcToolbar() override;

    void setAppEnvironment(state::StateMachine * stateMachine, wallet::Wallet * wallet );

    void updateButtonsState( state::STATE state );

protected:
    virtual void paintEvent(QPaintEvent *) override;

private slots:
    void on_airdropToolButton_clicked();

    void on_sendToolButton_clicked();

    void on_recieveToolButton_clicked();

    void on_transactionToolButton_clicked();

    void on_hodlToolButton_clicked();

    void onWalletBalanceUpdated();

private:
    Ui::MwcToolbar *ui;
    wallet::Wallet      * wallet = nullptr;
    state::StateMachine * stateMachine = nullptr;
};

}

#endif // MWCTOOLBAR_H

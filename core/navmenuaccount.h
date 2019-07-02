#ifndef NAVMENYACCOUNT_H
#define NAVMENYACCOUNT_H

#include <QWidget>
#include "navmenu.h"

namespace Ui {
class NavMenuAccount;
}


namespace state {
class StateMachine;
}

namespace core {

class NavMenuAccount : public NavMenu
{
    Q_OBJECT

public:
    explicit NavMenuAccount(QWidget *parent, state::StateMachine * stateMachine);
    ~NavMenuAccount();

private slots:
    void on_accountsButton_clicked();

    void on_seedButton_clicked();

    void on_contactsButton_clicked();

private:
    Ui::NavMenuAccount *ui;
    state::StateMachine * stateMachine = nullptr;
};

}

#endif // NAVMENYACCOUNT_H

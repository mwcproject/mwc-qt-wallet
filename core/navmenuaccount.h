#ifndef NAVMENYACCOUNT_H
#define NAVMENYACCOUNT_H

#include <QWidget>
#include "navmenu.h"

namespace Ui {
class NavMenuAccount;
}


namespace state {
struct StateContext;
}

namespace core {

class NavMenuAccount : public NavMenu
{
    Q_OBJECT

public:
    explicit NavMenuAccount(QWidget *parent, state::StateContext * context);
    ~NavMenuAccount();

private slots:
    void on_accountsButton_clicked();

    void on_seedButton_clicked();

    void on_contactsButton_clicked();

private:
    Ui::NavMenuAccount *ui;
    state::StateContext * context = nullptr;
};

}

#endif // NAVMENYACCOUNT_H

#ifndef NAVMENUCONFIG_H
#define NAVMENUCONFIG_H

#include <QWidget>
#include "navmenu.h"

namespace Ui {
class NavMenuConfig;
}


namespace state {
class StateMachine;
}

namespace core {

class NavMenuConfig : public NavMenu {
Q_OBJECT

public:
    explicit NavMenuConfig(QWidget *parent, state::StateMachine * stateMachine);

    ~NavMenuConfig();

private slots:
    void on_walletConfigButton_clicked();

    void on_outputsButton_clicked();

    void on_mwcmqButton_clicked();

private:
    Ui::NavMenuConfig *ui;
    state::StateMachine * stateMachine = nullptr;
};

}

#endif // NAVMENUCONFIG_H

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
    explicit NavMenuConfig(QWidget *parent, state::StateMachine * stateMachine, core::AppContext * appContext);

    ~NavMenuConfig();

private slots:
    void on_walletConfigButton_clicked();

    void on_outputsButton_clicked();

    void on_mwcmqButton_clicked();

    void on_resyncButton_clicked();

private:
    Ui::NavMenuConfig *ui;
    state::StateMachine * stateMachine = nullptr;
    core::AppContext * appContext = nullptr;
};

}

#endif // NAVMENUCONFIG_H

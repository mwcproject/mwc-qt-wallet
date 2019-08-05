#ifndef NAVMENUCONFIG_H
#define NAVMENUCONFIG_H

#include <QWidget>
#include "navmenu.h"

namespace Ui {
class NavMenuConfig;
}


namespace state {
struct StateContext;
}

namespace core {

class AppContext;

class NavMenuConfig : public NavMenu {
Q_OBJECT
public:
    explicit NavMenuConfig(QWidget *parent, state::StateContext * context );

    ~NavMenuConfig();

private slots:
    void on_walletConfigButton_clicked();

    void on_outputsButton_clicked();

    void on_mwcmqButton_clicked();

    void on_resyncButton_clicked();

    void on_nodeOverviewButton_clicked();

private:
    Ui::NavMenuConfig *ui;
    state::StateContext * context = nullptr;
};

}

#endif // NAVMENUCONFIG_H

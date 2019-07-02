#ifndef TOPRIGHTBUTTONS_H
#define TOPRIGHTBUTTONS_H

#include <QWidget>

namespace Ui {
class NavBar;
}

namespace state {
class StateMachine;
}

namespace core {

class NavMenu;

class NavBar : public QWidget {
Q_OBJECT

public:
    explicit NavBar(QWidget *parent, state::StateMachine * stateMachine);

    ~NavBar();

    void setNavMenuPosition( const QPoint & pos ) { navMenuPos = pos; }

private slots:
    void on_notificationButton_clicked();
    void on_settingsButton_clicked();
    void on_accountButton_clicked();

    void onMenuDestroy();
private:
    enum BTN {NONE, NOTIFICATION, SETTINGS, ACCOUNTS};
    void checkButton(BTN b);

    void showNavMenu( NavMenu * menu );

private:
    Ui::NavBar *ui;
    QWidget *prntWnd; // will be used for popup
    state::StateMachine * stateMachine = nullptr;
    QPoint navMenuPos; // Position of navigate menu
};

}

#endif // TOPRIGHTBUTTONS_H

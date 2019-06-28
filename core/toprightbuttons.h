#ifndef TOPRIGHTBUTTONS_H
#define TOPRIGHTBUTTONS_H

#include <QWidget>

namespace Ui {
class TopRightButtons;
}

namespace core {

class TopRightButtons : public QWidget {
Q_OBJECT

public:
    explicit TopRightButtons(QWidget *parent = nullptr);

    ~TopRightButtons();

private slots:
    void on_notificationButton_clicked();
    void on_settingsButton_clicked();
    void on_accountButton_clicked();
private:
    enum BTN {NOTIFICATION, SETTINGS, ACCOUNTS};
    void checkButton(BTN b);

private:
    Ui::TopRightButtons *ui;
    QWidget *prntWnd; // will be used for popup
};

}

#endif // TOPRIGHTBUTTONS_H

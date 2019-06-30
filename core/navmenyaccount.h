#ifndef NAVMENYACCOUNT_H
#define NAVMENYACCOUNT_H

#include <QWidget>

namespace Ui {
class NavMenyAccount;
}

class NavMenyAccount : public QWidget
{
    Q_OBJECT

public:
    explicit NavMenyAccount(QWidget *parent = nullptr);
    ~NavMenyAccount();

private:
    Ui::NavMenyAccount *ui;
};

#endif // NAVMENYACCOUNT_H

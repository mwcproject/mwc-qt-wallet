#ifndef NAVMENUCONFIG_H
#define NAVMENUCONFIG_H

#include <QWidget>

namespace Ui {
class NavMenuConfig;
}

class NavMenuConfig : public QWidget
{
    Q_OBJECT

public:
    explicit NavMenuConfig(QWidget *parent = nullptr);
    ~NavMenuConfig();

private:
    Ui::NavMenuConfig *ui;
};

#endif // NAVMENUCONFIG_H

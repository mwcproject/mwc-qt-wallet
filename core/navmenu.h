#ifndef NAVMENU_H
#define NAVMENU_H

#include <QWidget>
#include <QDialog>
#include <QFrame>

namespace core {

class NavMenu : public QFrame {
Q_OBJECT
public:
    explicit NavMenu(QWidget *parent = nullptr);

signals:
    void closed();

protected:
    virtual void focusOutEvent(QFocusEvent *event) override;

    virtual void leaveEvent(QEvent *event) override;
};

}

#endif // NAVMENU_H

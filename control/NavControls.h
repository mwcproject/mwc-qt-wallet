#ifndef MWC_QT_WALLET_NAVCONTROLS_H
#define MWC_QT_WALLET_NAVCONTROLS_H

#include <QPushButton>
#include <QToolButton>

// Navigation controls: Tool Button (NavControls) and menu button (NavMenuButton)
namespace control {

class NavPushButton : public QToolButton {
    Q_OBJECT
public:
    explicit NavPushButton(QWidget *parent = Q_NULLPTR);
    explicit NavPushButton(const QString &text, QWidget *parent = Q_NULLPTR);
    NavPushButton(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~NavPushButton() override;
};

class NavMenuButton : public QPushButton {
Q_OBJECT
public:
    explicit NavMenuButton(QWidget *parent = Q_NULLPTR);
    explicit NavMenuButton(const QString &text, QWidget *parent = Q_NULLPTR);
    NavMenuButton(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~NavMenuButton() override;
};

}

#endif //MWC_QT_WALLET_NAVCONTROLS_H

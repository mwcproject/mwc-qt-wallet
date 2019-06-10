#ifndef GUI_WALLET_MWCPUSHBUTTON_H
#define GUI_WALLET_MWCPUSHBUTTON_H

#include <QPushButton>

namespace control {

class MwcPushButtonNormal : public QPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonNormal(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonNormal(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonNormal(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonNormal() override;
};

class MwcPushButtonSmall : public QPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonSmall(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonSmall(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonSmall(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonSmall() override;
};

class MwcPushButtonTiny : public QPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonTiny(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonTiny(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonTiny(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonTiny() override;
};

}

#endif //GUI_WALLET_MWCPUSHBUTTON_H

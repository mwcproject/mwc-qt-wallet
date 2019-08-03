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

class MwcPushButtonTinyUnderline : public QPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonTinyUnderline(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonTinyUnderline(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonTinyUnderline(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonTinyUnderline() override;
};


class MwcPushButtonRound : public QPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonRound(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonRound(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonRound(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonRound() override;
};


}

#endif //GUI_WALLET_MWCPUSHBUTTON_H

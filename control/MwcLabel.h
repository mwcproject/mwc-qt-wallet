#ifndef GUI_WALLET_MWCLABEL_H
#define GUI_WALLET_MWCLABEL_H

#include <QLabel>

namespace control {

class MwcLabelTiny : public QLabel {
    Q_OBJECT
public:
    explicit MwcLabelTiny(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    explicit MwcLabelTiny(const QString &text, QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    virtual ~MwcLabelTiny() override;
};

class MwcLabelSmall : public QLabel {
    Q_OBJECT
public:
    explicit MwcLabelSmall(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    explicit MwcLabelSmall(const QString &text, QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    virtual ~MwcLabelSmall() override;
};

class MwcLabelNormal : public QLabel {
Q_OBJECT
public:
    explicit MwcLabelNormal(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    explicit MwcLabelNormal(const QString &text, QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    virtual ~MwcLabelNormal() override;
};

class MwcLabelLarge : public QLabel {
Q_OBJECT
public:
    explicit MwcLabelLarge(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    explicit MwcLabelLarge(const QString &text, QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    virtual ~MwcLabelLarge() override;
};

}

#endif //GUI_WALLET_MWCLABEL_H

#include "MwcPushButton.h"

namespace control {

// --------------  MwcPushButtonNormal  -------------------

MwcPushButtonNormal::MwcPushButtonNormal(QWidget *parent) :
        QPushButton(parent)
{}

MwcPushButtonNormal::MwcPushButtonNormal(const QString &text, QWidget *parent ) :
    QPushButton(text,parent)
{}

MwcPushButtonNormal::MwcPushButtonNormal(const QIcon &icon, const QString &text, QWidget *parent) :
    QPushButton(icon,text,parent)
{}

MwcPushButtonNormal::~MwcPushButtonNormal()
{}

// -----------------   MwcPushButtonSmall  -----------------------

MwcPushButtonSmall::MwcPushButtonSmall(QWidget *parent) :
        QPushButton(parent)
{}

MwcPushButtonSmall::MwcPushButtonSmall(const QString &text, QWidget *parent ) :
        QPushButton(text,parent)
{}

MwcPushButtonSmall::MwcPushButtonSmall(const QIcon &icon, const QString &text, QWidget *parent) :
        QPushButton(icon,text,parent)
{}

MwcPushButtonSmall::~MwcPushButtonSmall()
{}


// -----------------   MwcPushButtonTiny  -----------------------

MwcPushButtonTiny::MwcPushButtonTiny(QWidget *parent) :
        QPushButton(parent)
{}

MwcPushButtonTiny::MwcPushButtonTiny(const QString &text, QWidget *parent ) :
        QPushButton(text,parent)
{}

MwcPushButtonTiny::MwcPushButtonTiny(const QIcon &icon, const QString &text, QWidget *parent) :
        QPushButton(icon,text,parent)
{}

MwcPushButtonTiny::~MwcPushButtonTiny()
{}


// -----------------   MwcPushButtonTinyUnderline  -----------------------

MwcPushButtonTinyUnderline::MwcPushButtonTinyUnderline(QWidget *parent) :
        QPushButton(parent)
{}

MwcPushButtonTinyUnderline::MwcPushButtonTinyUnderline(const QString &text, QWidget *parent ) :
        QPushButton(text,parent)
{}

MwcPushButtonTinyUnderline::MwcPushButtonTinyUnderline(const QIcon &icon, const QString &text, QWidget *parent) :
        QPushButton(icon,text,parent)
{}

MwcPushButtonTinyUnderline::~MwcPushButtonTinyUnderline()
{}

// -----------------   MwcPushButtonRound  -----------------------


MwcPushButtonRound::MwcPushButtonRound(QWidget *parent) :
    QPushButton(parent)
{}
MwcPushButtonRound::MwcPushButtonRound(const QString &text, QWidget *parent) :
    QPushButton(text,parent)
{}

MwcPushButtonRound::MwcPushButtonRound(const QIcon &icon, const QString &text, QWidget *parent)  :
    QPushButton(icon,text,parent)
{}

MwcPushButtonRound::~MwcPushButtonRound() {}



}

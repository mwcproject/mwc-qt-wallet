#include "MwcLabel.h"

namespace control {

MwcLabelTiny::MwcLabelTiny(QWidget *parent, Qt::WindowFlags f) :
   QLabel(parent,f)
{}

MwcLabelTiny::MwcLabelTiny(const QString &text, QWidget *parent, Qt::WindowFlags f) :
    QLabel(text,parent, f)
{}

MwcLabelTiny::~MwcLabelTiny() {}


MwcLabelSmall::MwcLabelSmall(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent,f)
{}

MwcLabelSmall::MwcLabelSmall(const QString &text, QWidget *parent, Qt::WindowFlags f) :
    QLabel(text,parent, f)
{}

MwcLabelSmall::~MwcLabelSmall() {}


MwcLabelNormal::MwcLabelNormal(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent,f)
{}

MwcLabelNormal::MwcLabelNormal(const QString &text, QWidget *parent, Qt::WindowFlags f) :
        QLabel(text,parent, f)
{}

MwcLabelNormal::~MwcLabelNormal()
{}

MwcLabelLarge::MwcLabelLarge(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent,f)
{}

MwcLabelLarge::MwcLabelLarge(const QString &text, QWidget *parent, Qt::WindowFlags f) :
    QLabel(text,parent, f)
{}

MwcLabelLarge::~MwcLabelLarge()
{}

}

#include "MwcLineEdit.h"

namespace control {

MwcLineEditNormal::MwcLineEditNormal(QWidget *parent) :
    QLineEdit(parent)
{}

MwcLineEditNormal::MwcLineEditNormal(const QString & t, QWidget *parent ) :
        QLineEdit(t, parent)
{}

MwcLineEditNormal::~MwcLineEditNormal()
{}

}

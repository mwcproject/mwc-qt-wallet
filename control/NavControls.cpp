#include "NavControls.h"


namespace control {

// --------------  NavPushButton  -------------------

NavPushButton::NavPushButton(QWidget *parent) :
        QToolButton(parent) {}

NavPushButton::~NavPushButton() {}

// ----------------- NavMenuButton --------------------

NavMenuButton::NavMenuButton(QWidget *parent) :
        QPushButton(parent) {}

NavMenuButton::NavMenuButton(const QString &text, QWidget *parent) :
        QPushButton(text, parent) {}

NavMenuButton::NavMenuButton(const QIcon &icon, const QString &text, QWidget *parent) :
        QPushButton(icon, text, parent) {}

NavMenuButton::~NavMenuButton() {}


}
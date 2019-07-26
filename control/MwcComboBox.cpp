#include "MwcComboBox.h"

namespace control {

MwcComboBox::MwcComboBox(QWidget *parent) : QComboBox(parent) {}

void MwcComboBox::showPopup() {
    QComboBox::showPopup();
    QWidget *popup = this->findChild<QFrame *>();
    // Using pixels. Will work for used styles that defined in pixels too
    popup->move(popup->x() + 10, popup->y() + height());
}

}

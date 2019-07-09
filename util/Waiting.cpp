#include "Waiting.h"
#include <QApplication>

namespace util {

Waiting::Waiting() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

Waiting::~Waiting() {
    QApplication::restoreOverrideCursor();

}

}

#include "widgetutils.h"
#include <QWidget>
#include <QShortcut>

namespace utils {

void defineDefaultButtonSlot( QWidget *parent, const char *slot ) {
    // parent will take ownership
    new QShortcut( QKeySequence(Qt::Key_Return), parent, slot );
    new QShortcut( QKeySequence(Qt::Key_Enter), parent, slot );
}

}


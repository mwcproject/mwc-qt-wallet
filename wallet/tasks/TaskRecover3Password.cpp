#include "TaskRecover3Password.h"
#include <QDebug>
#include "../mwc713.h"
#include "TaskRecoverFull.h"

namespace wallet {

bool TaskRecover3Password::processTask(const QVector<WEvent> &events) {
    return ProcessRecoverTask(events, wallet713);
}


}


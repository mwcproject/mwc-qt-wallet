#include "mwc713task.h"

namespace wallet {

Mwc713Task::Mwc713Task(QString _taskName, QString _inputStr, MWC713 * _wallet713) :
    taskName(_taskName), wallet713(_wallet713), inputStr(_inputStr)
{
    Q_ASSERT(wallet713);
}

Mwc713Task::~Mwc713Task() {
}

// Filter events by type
QVector< WEvent > filterEvents(const QVector<WEvent> & events, WALLET_EVENTS type ) {
    QVector< WEvent > res;
    for (const auto & e : events) {
        if (e.event == type)
            res.push_back(e);
    }
    return res;
}

// Print events into the string
QString printEvents(const QVector<WEvent> & events) {
    QString res;

    for (const auto & e : events) {
        if (res.length()>0)
            res += ", ";

        res += "Evt(T=" + QString::number(e.event) + ", msg=" + e.message + ")";
    }
    return res;
}

}


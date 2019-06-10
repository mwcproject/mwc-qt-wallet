#ifndef GUI_WALLET_TASKRECOVER2MNENONIC_H
#define GUI_WALLET_TASKRECOVER2MNENONIC_H


#include "../mwc713task.h"
#include <QThread>

namespace wallet {

class TaskRecover2Mnenonic : public Mwc713Task {
public:
    const static long TIMEOUT = 2000; //

    // Expected that listening is already stopped
    TaskRecover2Mnenonic( MWC713 *wallet713, QVector<QString> seed ) :
            Mwc713Task("TaskRecover", calcCommand(seed), wallet713) {}

    virtual ~TaskRecover2Mnenonic() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;
private:
    QString calcCommand(QVector<QString> seed) const;

};

}




#endif //GUI_WALLET_TASKRECOVER2MNENONIC_H

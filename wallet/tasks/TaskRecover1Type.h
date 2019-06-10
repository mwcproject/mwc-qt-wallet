#ifndef GUI_WALLET_TASKRECOVER1TYPE_H
#define GUI_WALLET_TASKRECOVER1TYPE_H

#include "../mwc713task.h"
#include <QThread>

namespace wallet {

class TaskRecover1Type : public Mwc713Task {
public:
    const static long TIMEOUT = 1000; //

    // Expected that listening is already stopped
    TaskRecover1Type( MWC713 *wallet713 ) :
            Mwc713Task("TaskRecover", "recover", wallet713) {}

    virtual ~TaskRecover1Type() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;
};

}


#endif //GUI_WALLET_TASKRECOVER1TYPE_H

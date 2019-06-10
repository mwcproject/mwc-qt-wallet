#ifndef GUI_WALLET_TASKRECOVERPROGRESSLISTENER_H
#define GUI_WALLET_TASKRECOVERPROGRESSLISTENER_H


#include "../mwc713task.h"

namespace wallet {

// It is listener task. No input can be defined
class TaskRecoverProgressListener : public Mwc713Task {
public:
    const static long TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Start one listen per request. mwc713 doesn't support both
    TaskRecoverProgressListener( MWC713 *wallet713 ) :
            Mwc713Task("TaskRecoverProgressListener", "", wallet713) {}

    virtual ~TaskRecoverProgressListener() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;
};

}


#endif //GUI_WALLET_TASKRECOVERPROGRESSLISTENER_H

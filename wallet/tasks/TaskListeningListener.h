#ifndef GUI_WALLET_TASKLISTENINGLISTENER_H
#define GUI_WALLET_TASKLISTENINGLISTENER_H

#include "../mwc713task.h"

namespace wallet {

// It is listener task. No input can be defined.
// Listening for MWC MQ & keybase connection statuses
class TaskListeningListener : public Mwc713Task {
public:
    const static long TIMEOUT = 3600*1000*5; // 5 hours should be enough

    TaskListeningListener( MWC713 *wallet713 ) :
            Mwc713Task("TaskListeningListener", "", wallet713) {}

    virtual ~TaskListeningListener() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;
};

}


#endif //GUI_WALLET_TASKLISTENINGLISTENER_H

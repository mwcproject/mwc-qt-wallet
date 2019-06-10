#ifndef GUI_WALLET_TASKLISTENINGSTART_H
#define GUI_WALLET_TASKLISTENINGSTART_H

#include "../mwc713task.h"

namespace wallet {

class TaskListeningStart : public Mwc713Task {
public:
    const static long TIMEOUT = 7000;

    // Start one listen per request. mwc713 doesn't support both
    TaskListeningStart(MWC713 *wallet713, bool startMq, bool startKeybase) :
            Mwc713Task("TaskListeningStart", calcCommand(startMq, startKeybase), wallet713) {
        Q_ASSERT(startMq|startKeybase); Q_ASSERT( (startMq &&startKeybase) == false);
    }

    virtual ~TaskListeningStart() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

private:
    QString calcCommand(bool startMq, bool startKeybase) const;
};

}

#endif //GUI_WALLET_TASKLISTENINGSTART_H

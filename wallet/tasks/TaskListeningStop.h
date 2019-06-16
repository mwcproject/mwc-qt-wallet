#ifndef GUI_WALLET_TASKLISTENINGSTOP_H
#define GUI_WALLET_TASKLISTENINGSTOP_H


#include "../mwc713task.h"

namespace wallet {

class TaskListeningStop : public Mwc713Task {
public:
    const static long TIMEOUT = 8000;

    // Start one listen per request. mwc713 doesn't support both
    TaskListeningStop(MWC713 *wallet713, bool stopMq, bool stopKeybase) :
            Mwc713Task("TaskListeningStop", calcCommand(stopMq, stopKeybase), wallet713) {
        Q_ASSERT(stopMq | stopKeybase); Q_ASSERT( (stopMq && stopKeybase) == false);
    }

    virtual ~TaskListeningStop() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString calcCommand(bool stopMq, bool stopKeybase) const;
};

}


class TaskListeningStop {

};


#endif //GUI_WALLET_TASKLISTENINGSTOP_H

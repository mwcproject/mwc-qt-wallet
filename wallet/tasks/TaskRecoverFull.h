#ifndef GUI_WALLET_TASKRECOVERFULL_H
#define GUI_WALLET_TASKRECOVERFULL_H

#include "../mwc713task.h"

namespace wallet {

class TaskRecoverFull : public Mwc713Task {
public:
    const static long TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Expected that listening is already stopped
    TaskRecoverFull( MWC713 *wallet713, QVector<QString> seed, QString password ) :
            Mwc713Task("TaskRecover", calcCommand(seed, password ), wallet713) {}

    virtual ~TaskRecoverFull() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

private:
    QString calcCommand(QVector<QString> seed, QString password) const;
};

bool ProcessRecoverTask(const QVector<WEvent> &events,MWC713 *wallet713);

}


#endif //GUI_WALLET_TASKRECOVERFULL_H

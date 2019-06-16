#ifndef GUI_WALLET_TASKRECOVER3PASSWORD_H
#define GUI_WALLET_TASKRECOVER3PASSWORD_H

#include "../mwc713task.h"

namespace wallet {

// type password and start recovering. Might be pretty long process...
class TaskRecover3Password : public Mwc713Task {
public:
    const static long TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Expected that listening is already stopped
    TaskRecover3Password( MWC713 *wallet713, QString password ) :
            Mwc713Task("TaskRecover", password, wallet713) {}

    virtual ~TaskRecover3Password() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};

}

#endif //GUI_WALLET_TASKRECOVER3PASSWORD_H

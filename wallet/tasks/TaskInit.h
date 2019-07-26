#ifndef MWC_QT_WALLET_TASKINIT_H
#define MWC_QT_WALLET_TASKINIT_H

#include "../mwc713task.h"

namespace wallet {

class TaskInit : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 5000;

    TaskInit(MWC713 *wallet713) :
            Mwc713Task("TaskInit", "", wallet713, "") {} // no input for init

    virtual ~TaskInit() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual void onStarted() override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {
        return QSet<WALLET_EVENTS>{WALLET_EVENTS::S_INIT_WANT_ENTER};
    }

};

QVector<QString> calcSeedFromEvents(const QVector<WEvent> &events);


}

#endif //MWC_QT_WALLET_TASKINIT_H

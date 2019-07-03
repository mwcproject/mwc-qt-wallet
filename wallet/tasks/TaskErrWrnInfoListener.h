#ifndef GUI_WALLET_TASKERRWRNINFOLISTENER_H
#define GUI_WALLET_TASKERRWRNINFOLISTENER_H

#include "../mwc713task.h"

namespace wallet {

// istener: Listening for all Errors, Warnings and Infos
class TaskErrWrnInfoListener : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Start one listen per request. mwc713 doesn't support both
    TaskErrWrnInfoListener( MWC713 *wallet713 ) :
            Mwc713Task("TaskErrWrnInfoListener", "", wallet713,"") {}

    virtual ~TaskErrWrnInfoListener() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
};

}


#endif //GUI_WALLET_TASKERRWRNINFOLISTENER_H

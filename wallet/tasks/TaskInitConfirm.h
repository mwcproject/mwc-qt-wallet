#ifndef GUI_WALLET_TASKINITCONFIRM_H
#define GUI_WALLET_TASKINITCONFIRM_H

#include "../mwc713task.h"

namespace wallet {

class TaskInitConfirm : public Mwc713Task {
public:
    const static long TIMEOUT = 5000;

    TaskInitConfirm( MWC713 * wallet713) :
            Mwc713Task("TaskInitConfirm", " ", wallet713) {}  // Need to press enter only. But space is fine too

    virtual ~TaskInitConfirm() override;

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{
                WALLET_EVENTS::S_READY
    };}

};

}

#endif //GUI_WALLET_TASKINITCONFIRM_H

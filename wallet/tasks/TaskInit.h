#ifndef GUI_WALLET_TASKINIT_H
#define GUI_WALLET_TASKINIT_H

#include "../mwc713task.h"

namespace wallet {

class TaskInit : public Mwc713Task {
public:
    const static long TIMEOUT = 5000;

    TaskInit( MWC713 * wallet713) :
            Mwc713Task("TaskInit", "init", wallet713) {}

    virtual ~TaskInit() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

};

}

#endif //GUI_WALLET_TASKINIT_H

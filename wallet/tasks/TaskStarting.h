#ifndef GUI_WALLET_TASKSTARTING_H
#define GUI_WALLET_TASKSTARTING_H

#include "../mwc713task.h"

namespace wallet {

    class TaskStarting : public Mwc713Task {
    public:
        const static long TIMEOUT = 8000;

        TaskStarting( MWC713 * wallet713 ) : Mwc713Task("Starting", "", wallet713) {}
        virtual ~TaskStarting() override {}

        virtual bool processTask(const QVector<WEvent> & events) override;
    private:
    };

}

#endif //GUI_WALLET_TASKSTARTING_H

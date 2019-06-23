#ifndef MWC_QT_WALLET_TASKTRANSACTION_H
#define MWC_QT_WALLET_TASKTRANSACTION_H

#include "../mwc713task.h"

namespace wallet {

class TaskTransactions : public Mwc713Task {
public:
    const static long TIMEOUT = 1000*60;

    TaskTransactions( MWC713 * wallet713  ) :
            Mwc713Task("Transactions", "txs", wallet713, "") {}

    virtual ~TaskTransactions() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
    };

}

#endif //MWC_QT_WALLET_TASKTRANSACTION_H

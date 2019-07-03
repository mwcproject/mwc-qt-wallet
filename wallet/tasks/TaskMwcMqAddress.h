#ifndef GUI_WALLET_TASKADDRESS_H
#define GUI_WALLET_TASKADDRESS_H


#include "../mwc713task.h"

namespace wallet {

// Geting and chnanging
class TaskMwcMqAddress : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3000;

    // genNext:true - generate next address, address by index
    // idx     - index for generated address (genNext is true)
    // genNext:false - get current address with associated index
    TaskMwcMqAddress( MWC713 * wallet713, bool genNext, int idx ) :
            Mwc713Task("TaskMwcMqAddress", calcCommandLine(genNext, idx), wallet713,"") {}

    virtual ~TaskMwcMqAddress() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

private:
    QString calcCommandLine( bool genNext, int idx ) const;
};

}


#endif //GUI_WALLET_TASKADDRESS_H

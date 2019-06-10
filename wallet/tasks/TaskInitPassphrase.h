#ifndef GUI_WALLET_TASKINITPASSPHRASE_H
#define GUI_WALLET_TASKINITPASSPHRASE_H

#include "../mwc713task.h"

namespace wallet {

class TaskInitPassphrase : public Mwc713Task {
public:
    const static long TIMEOUT = 5000;

    TaskInitPassphrase( MWC713 * wallet713, QString password) :
            Mwc713Task("TaskInitPassphrase", password, wallet713) {}

    virtual ~TaskInitPassphrase() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

};

}

#endif //GUI_WALLET_TASKINITPASSPHRASE_H

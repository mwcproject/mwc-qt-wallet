#ifndef GUI_WALLET_TASKUNLOCK_H
#define GUI_WALLET_TASKUNLOCK_H

#include "../mwc713task.h"

namespace wallet {

    class TaskUnlock : public Mwc713Task {
    public:
        const static long TIMEOUT = 5000;

        TaskUnlock( MWC713 * wallet713, QString password, QString account ) :
                    Mwc713Task("Unlocking", buildWalletRequest(password, account), wallet713) {}

        virtual ~TaskUnlock() override {}

        virtual bool processTask(const QVector<WEvent> & events) override;
    private:
        static QString buildWalletRequest(QString password, QString account);
    };
}

#endif //GUI_WALLET_TASKUNLOCK_H

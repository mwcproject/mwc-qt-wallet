#ifndef GUI_WALLET_TASKUNLOCK_H
#define GUI_WALLET_TASKUNLOCK_H

#include "../mwc713task.h"

namespace wallet {

    class TaskUnlock : public Mwc713Task {
    public:
        const static int64_t TIMEOUT = 5000;

        TaskUnlock( MWC713 * wallet713, QString password) :
                    Mwc713Task("Unlocking", buildWalletRequest(password), wallet713, "unlock") {}

        virtual ~TaskUnlock() override {}

        virtual bool processTask(const QVector<WEvent> & events) override;

        virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_PASSWORD_ERROR, WALLET_EVENTS::S_READY };}

    private:
        static QString buildWalletRequest(QString password);
    };
}

#endif //GUI_WALLET_TASKUNLOCK_H

#ifndef GUI_WALLET_TASKINIT_H
#define GUI_WALLET_TASKINIT_H

#include "../mwc713task.h"

namespace wallet {

class TaskInit : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 5000;

    TaskInit( MWC713 * wallet713) :
            Mwc713Task("TaskInit", "init", wallet713, "") {}

    virtual ~TaskInit() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_PASSWORD_EXPECTED}; }

};

class TaskInitConfirm : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 5000;

    TaskInitConfirm( MWC713 * wallet713) :
            Mwc713Task("TaskInitConfirm", " ", wallet713,"") {}  // Need to press enter only. But space is fine too

    virtual ~TaskInitConfirm() override;

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{
                WALLET_EVENTS::S_READY
        };}

};

class TaskInitPassphrase : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 5000;

    TaskInitPassphrase( MWC713 * wallet713, QString password) :
            Mwc713Task("TaskInitPassphrase", password, wallet713, "password") {}

    virtual ~TaskInitPassphrase() override {}

    virtual void onStarted() override;

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{
                WALLET_EVENTS::S_INIT_WANT_ENTER, WALLET_EVENTS::S_READY
        };}

};


}

#endif //GUI_WALLET_TASKINIT_H

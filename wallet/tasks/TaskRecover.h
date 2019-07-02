#ifndef MWC_QT_WALLET_TASKRECOVER_H
#define MWC_QT_WALLET_TASKRECOVER_H

#include "../mwc713task.h"

namespace wallet {

// It is listener task. No input can be defined
class TaskRecoverProgressListener : public Mwc713Task {
public:
    const static long TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Start one listen per request. mwc713 doesn't support both
    TaskRecoverProgressListener( MWC713 *wallet713 ) :
            Mwc713Task("TaskRecoverProgressListener", "", wallet713,"") {}

    virtual ~TaskRecoverProgressListener() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}

};

/////////////////////////////////////////////////////

class TaskRecoverFull : public Mwc713Task {
public:
    const static long TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Expected that listening is already stopped
    TaskRecoverFull( MWC713 *wallet713, QVector<QString> seed, QString password ) :
            Mwc713Task("TaskRecover", calcCommand(seed, password ), wallet713, "recover from mnemonic") {}

    virtual ~TaskRecoverFull() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

private:
    QString calcCommand(QVector<QString> seed, QString password) const;
};


//////////////////////////////////////////////////////

class TaskRecover1Type : public Mwc713Task {
public:
    const static long TIMEOUT = 5000; //

    // Expected that listening is already stopped
    TaskRecover1Type( MWC713 *wallet713 ) :
            Mwc713Task("TaskRecover", "recover", wallet713,"") {}

    virtual ~TaskRecover1Type() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_RECOVERY_MNEMONIC };}

};

class TaskRecover2Mnenonic : public Mwc713Task {
public:
    const static long TIMEOUT = 5000; //

    // Expected that listening is already stopped
    TaskRecover2Mnenonic( MWC713 *wallet713, QVector<QString> seed ) :
            Mwc713Task("TaskRecover", calcCommand(seed), wallet713, "mnemonic seed") {}

    virtual ~TaskRecover2Mnenonic() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_PASSWORD_EXPECTED };}

private:
    QString calcCommand(QVector<QString> seed) const;

};

// type password and start recovering. Might be pretty long process...
class TaskRecover3Password : public Mwc713Task {
public:
    const static long TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Expected that listening is already stopped
    TaskRecover3Password( MWC713 *wallet713, QString password ) :
            Mwc713Task("TaskRecover", password, wallet713, "password") {}

    virtual ~TaskRecover3Password() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};

///////////////////////////////////////////////////////////////
////////////////////////

// Just show mnenonic
class TaskRecoverShowMnenonic : public Mwc713Task {
public:
    const static long TIMEOUT = 5000; //

    // Expected that listening is already stopped
    TaskRecoverShowMnenonic( MWC713 *wallet713, QString password ) :
            Mwc713Task("TaskRecover", "recover -d -p " + password, wallet713, "get mnemonic seed") {}

    virtual ~TaskRecoverShowMnenonic() override {}

    virtual void onStarted() override;

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

};


}



#endif //MWC_QT_WALLET_TASKRECOVER_H

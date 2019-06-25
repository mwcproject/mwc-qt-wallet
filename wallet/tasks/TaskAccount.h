#ifndef MWC_QT_WALLET_TASKACCOUNT_H
#define MWC_QT_WALLET_TASKACCOUNT_H

#include "../mwc713task.h"

namespace wallet {

// List all account
class TaskAccountList : public Mwc713Task {
public:
    const static long TIMEOUT = 1000*7;

    TaskAccountList( MWC713 *wallet713 ) :
            Mwc713Task("TaskAccountList", "accounts", wallet713,"") {}

    virtual ~TaskAccountList() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

};

// Create acoount command. Note that after account creation wallet keeping the original default account
class TaskAccountCreate : public Mwc713Task {
public:
    const static long TIMEOUT = 1000*7;

    // Expected that account is unique. Account must meet mwc713 expectations
    TaskAccountCreate( MWC713 * wallet713, QString account ) :
            Mwc713Task("TaskAccountCreate", "account create " + account, wallet713,""), newAccountName(account) { Q_ASSERT(account.length()>0); }

    virtual ~TaskAccountCreate() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString newAccountName;
};

class TaskAccountSwitch : public Mwc713Task {
public:
    const static long TIMEOUT = 1000*7;

    TaskAccountSwitch( MWC713 * wallet713, QString accountName, QString password ) :
            Mwc713Task("TaskAccountSwitch", "account switch " + accountName + " -p " + password, wallet713, "account switch " + accountName ),
            switchAccountName(accountName) { Q_ASSERT(accountName.length()>0); Q_ASSERT(password.length()>0); }

    virtual ~TaskAccountSwitch() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString switchAccountName;
};


class TaskAccountInfo : public Mwc713Task {
public:
    const static long TIMEOUT = 1000*10;

    // noRefresh can be used for sequenced calls
    TaskAccountInfo( MWC713 * wallet713, bool noRefresh ) :
    Mwc713Task("TaskAccountInfo", QString("info") + (noRefresh ? " --no-refresh":""), wallet713,"") {}

    virtual ~TaskAccountInfo() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};

// Just a callback, not a real task
class TaskAccountProgress : public Mwc713Task {
public:
    TaskAccountProgress( MWC713 * wallet713, int _pos, int _total ) :
    Mwc713Task("TaskAccountProgress", "", wallet713,""), pos(_pos), total(_total) {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
private:
    int pos;
    int total;
};

// Just a callback, not a real task
class TaskAccountListFinal : public Mwc713Task {
public:
    TaskAccountListFinal( MWC713 * wallet713, QString _accountName2switch ) :
            Mwc713Task("TaskAccountListFinal", "", wallet713,""), accountName2switch(_accountName2switch) {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
private:
    QString accountName2switch;
};

}

#endif //MWC_QT_WALLET_TASKACCOUNT_H
